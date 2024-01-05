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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//���¿ͻ����ϵĵ÷�
	virtual void OnRep_Score() override;
	//���·������ϵĵ÷�
	void AddToScore(float ScoreAmount);

	//����������
	UFUNCTION()
		virtual void OnRep_Defeats();
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
		class AXCharacter* XCharacter;
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
