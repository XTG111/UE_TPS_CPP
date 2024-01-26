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

	//统计一局游戏得分最高的玩家利用PlayerState
	UPROPERTY(Replicated)
		TArray<AXBlasterPlayerState*> TopScoringPlayers;
	
	/*Teams*/

	//对队伍分数的修改
	void AddRedTeamScore();
	void AddBlueTeamScore();

	//统计最高得分团队
	UPROPERTY()
		TArray<AXBlasterPlayerState*> RedTeam;
	UPROPERTY()
		TArray<AXBlasterPlayerState*> BlueTeam;

	//团队分数
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
		float RedTeamScore = 0.f;
	UFUNCTION()
		void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
		float BlueTeamScore = 0.f;
	UFUNCTION()
		void OnRep_BlueTeamScore();

private:
	//最高分
	float TopScore = 0.f;
	
};
