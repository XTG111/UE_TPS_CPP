// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionSubsystem.h"
#include "MatchType.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GetGameState<AGameModeBase>())
	{
		AGameModeBase* SelfGameState = GetGameState<AGameModeBase>();
		int32 NumOfPlayer = SelfGameState->GetNumPlayers();

		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UMultiplayerSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
			check(Subsystem);
			//当连接人数达到我们自定义的人数进行跳转
			if (NumOfPlayer == Subsystem->GetDesiredNumPublicConnections())
			{
				UWorld* World = GetWorld();
				if (World)
				{
					//seamlessTravel
					bUseSeamlessTravel = true;
					FString MatchType = Subsystem->GetDesireMatchType();
					if (MatchType == "FreeForAll")
					{
						World->ServerTravel(FString("/Game/Map/XBlaster_Map?listen"));
					}
					else if (MatchType == "Teams")
					{
						World->ServerTravel(FString("/Game/Map/XBlaster_Map?listen"));
					}
					else if (MatchType == "CTF")
					{
						World->ServerTravel(FString("/Game/Map/XBlaster_Map?listen"));
					}
				}
			}
		}
	}
}
