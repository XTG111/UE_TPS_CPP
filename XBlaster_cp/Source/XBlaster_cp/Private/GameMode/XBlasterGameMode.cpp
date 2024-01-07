// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/XBlasterGameMode.h"
#include "Character/XCharacter.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "GameState/XBlasterGameState.h"

namespace MatchState
{
	const FName CoolDown = FName("CoolDown");
}

AXBlasterGameMode::AXBlasterGameMode()
{
	bDelayedStart = true;
}

void AXBlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AXBlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(*It);
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
}



void AXBlasterGameMode::Tick(float DeltatTime)
{
	Super::Tick(DeltatTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::CoolDown);
		}
	}
	else if (MatchState == MatchState::CoolDown)
	{
		CountDownTime = CoolDownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AXBlasterGameMode::PlayerEliminated(AXCharacter* ElimmedCharacter, AXBlasterPlayerController* VictimController, AXBlasterPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
	//当玩家被击败，获取击杀玩家的控制器利用控制器获取PlayerState
	AXBlasterPlayerState* AttackPlayerState = AttackerController ? Cast<AXBlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	AXBlasterPlayerState* VictimPlayerState = VictimController ? Cast<AXBlasterPlayerState>(VictimController->PlayerState) : nullptr;
	
	//GameState
	AXBlasterGameState* XBlasterGameState = GetGameState < AXBlasterGameState >();

	if (AttackPlayerState && AttackPlayerState != VictimPlayerState && XBlasterGameState)
	{
		AttackPlayerState->AddToScore(1.f);
		XBlasterGameState->UpdateTopScore(AttackPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
}

void AXBlasterGameMode::RequestRespawn(AXCharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}