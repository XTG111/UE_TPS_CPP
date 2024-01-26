// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/XBlasterGameMode.h"
#include "XTeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXTeamGameMode : public AXBlasterGameMode
{
	GENERATED_BODY()
public:
	AXTeamGameMode();
	//中途加入的玩家确定队伍划分
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//玩家离开时踢出队伍
	virtual void Logout(AController* Exiting) override;
	//计算友伤
	virtual float CalculateDamage(AController* Attacker, AController* Vicitim, float BaseDamage) override;
	//玩家死亡
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController) override;

protected:
	//游戏开始时加入的玩家确定队伍划分
	virtual void HandleMatchHasStarted() override;

private:
	UPROPERTY(EditAnywhere)
		bool bCanFireFriend = false;
	
};
