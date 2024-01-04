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
	//���¿ͻ����ϵĵ÷�
	virtual void OnRep_Score() override;
	//���·������ϵĵ÷�
	void AddToScore(float ScoreAmount);

private:
	class AXCharacter* XCharacter;
	class AXBlasterPlayerController* XBlasterPlayerController;

	
};
