// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/XBlasterGameMode.h"
#include "MultiplayerSessionSubsystem.h"
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
			XBlasterPlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
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
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
	//当玩家被击败，获取击杀玩家的控制器利用控制器获取PlayerState
	AXBlasterPlayerState* AttackPlayerState = AttackerController ? Cast<AXBlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	AXBlasterPlayerState* VictimPlayerState = VictimController ? Cast<AXBlasterPlayerState>(VictimController->PlayerState) : nullptr;

	//GameState
	AXBlasterGameState* XBlasterGameState = GetGameState < AXBlasterGameState >();

	if (AttackPlayerState && AttackPlayerState != VictimPlayerState && XBlasterGameState)
	{
		TArray<AXBlasterPlayerState*> PlayersCurrentlyIntheLead;
		for (auto LeadPlayer : XBlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyIntheLead.Add(LeadPlayer);
		}

		AttackPlayerState->AddToScore(1.f);
		XBlasterGameState->UpdateTopScore(AttackPlayerState);
		if (XBlasterGameState->TopScoringPlayers.Contains(AttackPlayerState))
		{
			AXCharacter* LeadCharacter = Cast<AXCharacter>(AttackPlayerState->GetPawn());
			if (LeadCharacter)
			{
				LeadCharacter->MulticastGainerTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyIntheLead.Num(); i++)
		{
			if (!XBlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyIntheLead[i]))
			{
				AXCharacter* LosCharacter = Cast<AXCharacter>(PlayersCurrentlyIntheLead[i]->GetPawn());
				if (LosCharacter)
				{
					LosCharacter->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	//遍历所有的玩家控制器，然后调用自己的BroadcastElimAnnouncement函数
	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; it++)
	{
		AXBlasterPlayerController* XBlasterPlayerController = Cast<AXBlasterPlayerController>(*it);
		if (XBlasterPlayerController && AttackPlayerState && VictimPlayerState)
		{
			XBlasterPlayerController->BroadcastElim(AttackPlayerState, VictimPlayerState);
		}
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

void AXBlasterGameMode::PlayerLeftGame(AXBlasterPlayerState* LeavingPlayerState)
{
	//TODO 向服务器上的玩家死亡函数，传入bLeftGame = true
	//检查TopPlayer是否包含这个离开的角色
	if (LeavingPlayerState == nullptr) return;
	AXBlasterGameState* XBlasterGameState = GetGameState < AXBlasterGameState>();
	if (XBlasterGameState && XBlasterGameState->TopScoringPlayers.Contains(LeavingPlayerState))
	{
		XBlasterGameState->TopScoringPlayers.Remove(LeavingPlayerState);
	}
	//设置bLeftGame为true然后不再重生
	AXCharacter* CharacterLeaving = Cast<AXCharacter>(LeavingPlayerState->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

float AXBlasterGameMode::CalculateDamage(AController* Attacker, AController* Vicitim, float BaseDamage)
{
	return BaseDamage;
}
