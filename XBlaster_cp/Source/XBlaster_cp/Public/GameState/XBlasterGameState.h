// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "XBlasterGameState.generated.h"


class AXBlasterPlayerState;
/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(AXBlasterPlayerState* ScoringPlayers);

	//ͳ��һ����Ϸ�÷���ߵ��������PlayerState
	UPROPERTY(Replicated)
		TArray<AXBlasterPlayerState*> TopScoringPlayers;

private:
	//��߷�
	float TopScore = 0.f;
	
};
