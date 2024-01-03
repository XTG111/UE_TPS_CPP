// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XBlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	//角色死亡
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController);
	//重生角色
	virtual void RequestRespawn(class AXCharacter* ElimmedCharacter, AController* ElimmedController);
};
