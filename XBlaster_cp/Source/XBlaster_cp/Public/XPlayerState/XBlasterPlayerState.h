// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "XBlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	//更新客户端上的得分
	virtual void OnRep_Score() override;
	//更新服务器上的得分
	void AddToScore(float ScoreAmount);

private:
	class AXCharacter* XCharacter;
	class AXBlasterPlayerController* XBlasterPlayerController;

	
};
