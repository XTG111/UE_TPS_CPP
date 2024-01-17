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
	//virtual void Fire(const FVector& HitTarget) override;

	//专门为霰弹枪这种多子弹的武器编写击中，目标检测
	virtual void FireShotGun(const TArray<FVector_NetQuantize>& HitTargets);

	/*霰弹枪的命中点统计传播
	*/
	void ShotGunTraceEndwithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
private:
	//霰弹枪子弹数
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
		uint32 NumberOfPellets = 10;
	
};
