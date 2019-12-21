// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Weapons/ShooterWeapon_StickyGrenade.h"
#include "Weapons/ShooterProjectile.h"
#include "ShooterStickyGrenade.h"

AShooterWeapon_StickyGrenade::AShooterWeapon_StickyGrenade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AShooterWeapon_StickyGrenade::FireWeapon()
{
	FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();

	ServerFireProjectile(Origin, ShootDir);
}

bool AShooterWeapon_StickyGrenade::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AShooterWeapon_StickyGrenade::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AShooterStickyGrenade* Grenade = Cast<AShooterStickyGrenade>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, GrenadeConfig.GrenadeClass, SpawnTM));
	if (Grenade)
	{
		Grenade->Instigator = Instigator;
		Grenade->SetOwner(this);
		Grenade->Server_MoveToTarget(ShootDir);

		UGameplayStatics::FinishSpawningActor(Grenade, SpawnTM);
	}
}

void AShooterWeapon_StickyGrenade::ApplyWeaponConfig(FStickyGrenadeWeaponData& Data)
{
	Data = GrenadeConfig;
}
