// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPGameDemoGameMode.h"
#include "MPGameDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMPGameDemoGameMode::AMPGameDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
