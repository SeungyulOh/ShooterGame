// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterWeapon.h"
#include "GameFramework/DamageType.h" // for UDamageType::StaticClass()
#include "ShooterWeapon_StickyGrenade.generated.h"

USTRUCT()
struct FStickyGrenadeWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AShooterStickyGrenade> GrenadeClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	float ProjectileLife;

	/** Bounce count to explode */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 MaxBounceCount;

	/** after stop moving, time-delay to explode */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeDelay;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FStickyGrenadeWeaponData()
	{
		GrenadeClass = nullptr;
		MaxBounceCount = 2;
		TimeDelay = 3.f;
		ProjectileLife = 5.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

// A weapon that fires a visible projectile
UCLASS(Abstract)
class AShooterWeapon_StickyGrenade : public AShooterWeapon
{
	GENERATED_UCLASS_BODY()

	/** apply config on projectile */
	void ApplyWeaponConfig(FStickyGrenadeWeaponData& Data);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EGrenade;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FStickyGrenadeWeaponData GrenadeConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);
};
