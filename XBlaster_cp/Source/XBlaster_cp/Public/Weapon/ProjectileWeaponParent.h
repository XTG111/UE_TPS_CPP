// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponParent.h"
#include "ProjectileWeaponParent.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AProjectileWeaponParent : public AWeaponParent
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectileActor> ProjectileClass;
	
};
