// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponParent.h"
#include "HitScanWeaponParent.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AHitScanWeaponParent : public AWeaponParent
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
		UParticleSystem* BeamParticles;

	//chongfengqiang
	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;
	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	//处理冲锋枪效果
	void SetSubMachineGunProper(UWorld* World, FTransform& SockertTransform);
	
};
