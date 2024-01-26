// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/XBlasterGameState.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/XBlasterPlayerController.h"

void AXBlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(AXBlasterGameState, RedTeamScore);
	DOREPLIFETIME(AXBlasterGameState, BlueTeamScore);
}

//更新最高分玩家
void AXBlasterGameState::UpdateTopScore(AXBlasterPlayerState* ScoringPlayers)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayers);
		TopScore = ScoringPlayers->GetScore();
	}
	else if(ScoringPlayers->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayers);
	}
	else if (ScoringPlayers->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayers);
		TopScore = ScoringPlayers->GetScore();
	}
}

void AXBlasterGameState::AddRedTeamScore()
{
	++RedTeamScore;
	AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDRedTeamScores(RedTeamScore);
	}
}

void AXBlasterGameState::AddBlueTeamScore()
{
	++BlueTeamScore;
	AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDBlueTeamScores(BlueTeamScore);
	}
}

void AXBlasterGameState::OnRep_RedTeamScore()
{
	AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDRedTeamScores(RedTeamScore);
	}
}

void AXBlasterGameState::OnRep_BlueTeamScore()
{
	AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDBlueTeamScores(BlueTeamScore);
	}
}
