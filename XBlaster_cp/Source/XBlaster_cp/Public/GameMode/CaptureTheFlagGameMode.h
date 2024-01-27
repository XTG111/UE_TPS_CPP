// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/XTeamGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API ACaptureTheFlagGameMode : public AXTeamGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController) override;
	
	//º∆À„µ√∑÷
	void FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone);
};
