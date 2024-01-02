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
	void SetHealth(float Health, float MaxHealth);

protected:
	virtual void BeginPlay() override;

private:

	//利用玩家控制器来设置HUD
	class AXBlasterHUD* XBlasterHUD;
	
};
