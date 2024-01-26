// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/XTeamGameMode.h"
#include "GameState/XBlasterGameState.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"

AXTeamGameMode::AXTeamGameMode()
{
	bTeamsMatch = true;
}

void AXTeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AXBlasterGameState* BGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		//新加入的玩家加入队伍
		AXBlasterPlayerState* XBlasterPlayerState = NewPlayer->GetPlayerState<AXBlasterPlayerState>();
		if (XBlasterPlayerState && XBlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(XBlasterPlayerState);
				XBlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(XBlasterPlayerState);
				XBlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void AXTeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	AXBlasterGameState* BGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState && Exiting)
	{
		//玩家离开游戏
		AXBlasterPlayerState* XBlasterPlayerState = Exiting->GetPlayerState<AXBlasterPlayerState>();
		if (XBlasterPlayerState)
		{
			if (BGameState->RedTeam.Contains(XBlasterPlayerState))
			{
				BGameState->RedTeam.Remove(XBlasterPlayerState);
			}
			if (BGameState->BlueTeam.Contains(XBlasterPlayerState))
			{
				BGameState->BlueTeam.Remove(XBlasterPlayerState);
			}
		}
	}
}

float AXTeamGameMode::CalculateDamage(AController* Attacker, AController* Vicitim, float BaseDamage)
{
	AXBlasterPlayerState* AttackerState = Attacker->GetPlayerState<AXBlasterPlayerState>();
	AXBlasterPlayerState* VicitimState = Vicitim->GetPlayerState<AXBlasterPlayerState>();
	if (AttackerState == nullptr || VicitimState == nullptr) return BaseDamage;
	if (VicitimState == AttackerState) return BaseDamage;
	if (AttackerState->GetTeam() == VicitimState->GetTeam())
	{
		float Damage = bCanFireFriend ? BaseDamage : 0.f;
		return Damage;
	}
	return BaseDamage;
}

void AXTeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	//分组
	AXBlasterGameState* BGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PlayerState : BGameState->PlayerArray)
		{
			AXBlasterPlayerState* XBlasterPlayerState = Cast<AXBlasterPlayerState>(PlayerState);
			if (XBlasterPlayerState && XBlasterPlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(XBlasterPlayerState);
					XBlasterPlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(XBlasterPlayerState);
					XBlasterPlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

void AXTeamGameMode::PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	AXBlasterGameState* BGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
	AXBlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<AXBlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	AXBlasterPlayerState* VictimPlayerState = VictimController ? Cast<AXBlasterPlayerState>(VictimController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState && VictimPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam && VictimPlayerState->GetTeam() != ETeam::ET_BlueTeam)
		{
			BGameState->AddBlueTeamScore();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam && VictimPlayerState->GetTeam() != ETeam::ET_RedTeam)
		{
			BGameState->AddRedTeamScore();
		}
	}
}
