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

	//��ɫ����
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController);
	//������ɫ
	virtual void RequestRespawn(class AXCharacter* ElimmedCharacter, AController* ElimmedController);
};
