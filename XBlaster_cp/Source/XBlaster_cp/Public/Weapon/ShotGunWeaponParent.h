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

	//ר��Ϊ����ǹ���ֶ��ӵ���������д���У�Ŀ����
	virtual void FireShotGun(const TArray<FVector_NetQuantize>& HitTargets);

	/*����ǹ�����е�ͳ�ƴ���
	*/
	void ShotGunTraceEndwithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
	//ͳ�Ʊ����еĽ�ɫ
	UPROPERTY()
		TArray<AXCharacter*> HitCharacters;
private:
	//����ǹ�ӵ���
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
		uint32 NumberOfPellets = 10;

	//ͳ�ƻ���ͷ������������ɵ��˺�
	TMap<AXCharacter*, uint32> GetDamageMap(TMap<AXCharacter*, uint32>& HitMap, TMap<AXCharacter*, uint32>& HeadShotHitMap);
	
};
