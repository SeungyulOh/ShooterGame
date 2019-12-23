// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterStickyGrenade.h"
#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterExplosionEffect.h"
#include "Components/StaticMeshComponent.h"

AShooterStickyGrenade::AShooterStickyGrenade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CollisionComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;
	
	StaticMeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("StaticMeshComp"));
	StaticMeshComp->AlwaysLoadOnServer = false;
	StaticMeshComp->AlwaysLoadOnClient = true;
	StaticMeshComp->SetupAttachment(RootComponent);

	ParticleComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = false;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->AlwaysLoadOnServer = false;
	ParticleComp->AlwaysLoadOnClient = true;
	ParticleComp->SetupAttachment(RootComponent);

	MovementComp = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 1.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateMovement = true;
}

void AShooterStickyGrenade::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		if (MovementComp)
		{
			MovementComp->OnProjectileBounce.AddDynamic(this, &AShooterStickyGrenade::Server_OnBounce);
		}
	}
	
	CollisionComp->MoveIgnoreActors.Add(Instigator);
	
	AShooterWeapon_StickyGrenade* OwnerWeapon = Cast<AShooterWeapon_StickyGrenade>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}

	SetLifeSpan( WeaponConfig.ProjectileLife );
	MyController = GetInstigatorController();
}

void AShooterStickyGrenade::Server_MoveToTarget(FVector& ShootDirection)
{
	if (MovementComp)
	{
		MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
		State = EGrenadeState::MoveToTarget;
	}
}

void AShooterStickyGrenade::Server_StopMove(const FHitResult& HitResult)
{
	UWorld* CurrentWorld = GetWorld();
	if (CurrentWorld == nullptr)
		return;

	if (MovementComp)
	{
		MovementComp->StopMovementImmediately();
		MovementComp->ProjectileGravityScale = 0.f;
	}

	State = EGrenadeState::StopMove;

	if (!CurrentWorld->GetTimerManager().TimerExists(TimerHandle))
	{
		auto Callback_Timer = [this, HitResult]()
		{
			Server_Explode(HitResult);
		};
		CurrentWorld->GetTimerManager().SetTimer(TimerHandle, Callback_Timer, WeaponConfig.TimeDelay, false);
	}
	
	//In case of running as ListenServer or Standalone,
	//Client_StopMove should be executed necessarily.
	if (IsNetMode(NM_ListenServer) || IsNetMode(NM_Standalone))
	{
		Client_StopMove();
	}
}

void AShooterStickyGrenade::Server_Explode(const FHitResult& Impact)
{
	// effects and damage origin shouldn't be placed inside mesh at impact point
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, NudgedImpactLocation, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}

	State = EGrenadeState::Explosion;
	SetLifeSpan(2.0f);

	//In case of running as ListenServer or Standalone,
	//Client_Explode should be executed necessarily.
	if (IsNetMode(NM_ListenServer) || IsNetMode(NM_Standalone))
	{
		Client_Explode();
	}
}

void AShooterStickyGrenade::Server_OnBounce(const FHitResult& HitResult, const FVector& ImpactVelocity)
{
	WeaponConfig.MaxBounceCount--;
	if (WeaponConfig.MaxBounceCount <= 0)
	{
		Server_StopMove(HitResult);
	}
}



void AShooterStickyGrenade::Client_StopMove()
{
	if (MovementComp)
	{
		MovementComp->StopMovementImmediately();
		MovementComp->ProjectileGravityScale = 0.f;
	}

	if (ParticleComp)
	{
		ParticleComp->Activate();
	}
}

void AShooterStickyGrenade::Client_Explode()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	if (StaticMeshComp)
	{
		StaticMeshComp->SetHiddenInGame(true);
	}

	FVector ProjDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(TEXT("ProjClient"), true, Instigator)))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		if (IsNetMode(NM_Client))
		{
			UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, NudgedImpactLocation, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
		}
	}

	if (ExplosionTemplate)
	{
		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), NudgedImpactLocation);
		AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
}

///CODE_SNIPPET_START: AActor::GetActorLocation AActor::GetActorRotation

void AShooterStickyGrenade::OnRep_StateChanged()
{
	switch (State)
	{
	case EGrenadeState::Spawned:
		break;

	case EGrenadeState::MoveToTarget:
		break;

	case EGrenadeState::StopMove:
		Client_StopMove();
		break;

	case EGrenadeState::Explosion:
		Client_Explode();
		break;

	default:
		break;
	}
}

///CODE_SNIPPET_END

void AShooterStickyGrenade::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AShooterStickyGrenade::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME(AShooterStickyGrenade, State );
}