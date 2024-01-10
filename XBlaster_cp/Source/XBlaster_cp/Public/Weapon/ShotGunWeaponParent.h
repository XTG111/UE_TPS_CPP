// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeaponParent.h"
#include "ShotGunWeaponParent.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AShotGunWeaponParent : public AHitScanWeaponParent
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
private:
	//ö±µ¯Ç¹×Óµ¯Êý
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
		uint32 NumberOfPellets = 10;
	
};
