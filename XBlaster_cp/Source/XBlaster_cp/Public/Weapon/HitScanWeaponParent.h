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

	//处理冲锋枪效果
	void SetSubMachineGunProper(UWorld* World, FTransform& SockertTransform);

	/*
	Trace End with scatter
	*/
	//散步距离
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	//散步球体半径
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	//是否开启散布
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseScatter = false;

protected:

	//计算得到散步的一点
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	//整合计算射线检测目标的功能
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
