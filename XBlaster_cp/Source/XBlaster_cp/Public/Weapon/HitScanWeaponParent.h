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

	//chongfengqiang
	UPROPERTY(EditAnywhere)
		UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;

	//������ǹЧ��
	void SetSubMachineGunProper(UWorld* World, FTransform& SockertTransform);
protected:


	//���ϼ������߼��Ŀ��Ĺ���
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticle;
	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;
	UPROPERTY(EditAnywhere)
		float Damage = 20.f;
	UPROPERTY(EditAnywhere)
		class UParticleSystem* BeamParticles;	
};
