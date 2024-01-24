// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeaponParent.h"
#include "SnipperWeaponParent.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API ASnipperWeaponParent : public AHitScanWeaponParent
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	void SnipperTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	//
	UPROPERTY(VisibleAnywhere)
		bool bUnderAiming;
	
};
