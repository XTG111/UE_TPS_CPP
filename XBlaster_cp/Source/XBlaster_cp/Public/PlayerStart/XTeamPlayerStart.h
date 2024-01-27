// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "XBlaster_cp/XTypeHeadFile/TeamState.h"
#include "XTeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		ETeam TeamType;
	
};
