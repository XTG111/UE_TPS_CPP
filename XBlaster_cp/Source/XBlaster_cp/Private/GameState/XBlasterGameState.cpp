// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/XBlasterGameState.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void AXBlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBlasterGameState, TopScoringPlayers);
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
