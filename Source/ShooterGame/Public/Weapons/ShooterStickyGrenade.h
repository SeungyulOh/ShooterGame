// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ShooterWeapon_StickyGrenade.h"
#include "ShooterStickyGrenade.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EGrenadeState : uint8
{
	Spawned,
	MoveToTarget,
	StopMove,
	Explosion,
	End
};

// 
UCLASS(Abstract, Blueprintable)
class AShooterStickyGrenade : public AActor
{
	GENERATED_UCLASS_BODY()

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** Server MoveToTarget State */
	void Server_MoveToTarget(FVector& ShootDirection);
	/** Server StopMove State */
	void Server_StopMove(const FHitResult& HitResult);
	/** Server Explosion State */
	void Server_Explode(const FHitResult& Impact);

	void Client_StopMove();
	void Client_Explode();

	/** [server]handle hit */
	UFUNCTION()
	void Server_OnBounce(const FHitResult& HitResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void OnRep_StateChanged();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class AShooterExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	struct FStickyGrenadeWeaponData WeaponConfig;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_StateChanged)
	EGrenadeState State = EGrenadeState::Spawned;

	FTimerHandle TimerHandle;
private:
	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Grenade)
	UStaticMeshComponent* StaticMeshComp;

	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Grenade)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Grenade)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Grenade)
	UParticleSystemComponent* ParticleComp;

	

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UStaticMeshComponent* GetStaticMesh() const { return StaticMeshComp; }
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
};
