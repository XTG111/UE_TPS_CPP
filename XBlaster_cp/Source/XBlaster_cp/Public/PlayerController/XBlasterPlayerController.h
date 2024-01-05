// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XBlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//设置Health
	void SetHUDHealth(float Health, float MaxHealth);

	void OnPossess(APawn* InPawn) override;

	//击杀数
	void SetHUDScore(float Score);
	//死亡数
	void SetHUDDefeats(int32 Defeats);
	//子弹数量
	void SetHUDWeaponAmmo(int32 Ammo);
	//备弹数
	void SetHUDCarriedAmmo(int32 Ammo);

protected:
	virtual void BeginPlay() override;

private:

	//利用玩家控制器来设置HUD
	class AXBlasterHUD* XBlasterHUD;
	
};
