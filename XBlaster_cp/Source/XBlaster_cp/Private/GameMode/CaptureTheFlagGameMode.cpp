// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CaptureTheFlagGameMode.h"
#include "FlagActor/FlagZone.h"
#include "Weapon/Flag.h"
#include "GameState/XBlasterGameState.h"
#include "Kismet/GameplayStatics.h"

void ACaptureTheFlagGameMode::PlayerEliminated(AXCharacter* ElimmedCharacter, AXBlasterPlayerController* VictimController, AXBlasterPlayerController* AttackerController)
{
	AXBlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->TeamType != FlagZone->TeamType;
	AXBlasterGameState* BGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		if (FlagZone->TeamType == ETeam::ET_BlueTeam)
		{
			BGameState->AddBlueTeamScore();
		}
		if (FlagZone->TeamType == ETeam::ET_RedTeam)
		{
			BGameState->AddRedTeamScore();
		}
	}
}
