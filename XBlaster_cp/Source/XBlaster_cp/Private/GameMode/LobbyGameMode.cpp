// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GetGameState<AGameModeBase>())
	{
		AGameModeBase* SelfGameState = GetGameState<AGameModeBase>();
		int32 NumOfPlayer = SelfGameState->GetNumPlayers();
		if (NumOfPlayer == 2)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				//seamlessTravel
				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Map/Lobby?listen"));
			}
		}
	}
	
}
