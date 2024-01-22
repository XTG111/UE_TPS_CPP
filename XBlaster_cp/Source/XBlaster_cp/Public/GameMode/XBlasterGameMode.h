// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XBlasterGameMode.generated.h"

//增加MatchState
namespace MatchState
{
	extern XBLASTER_CP_API const FName CoolDown;//游戏结束，显示获胜玩家，并且开启显示结束缓冲时间
}

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	//设置延迟启动利用构造函数
	AXBlasterGameMode();

	virtual void Tick(float DeltatTime) override;

	//角色死亡
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController);
	//重生角色
	virtual void RequestRespawn(class AXCharacter* ElimmedCharacter, AController* ElimmedController);
	//角色退出游戏
	void PlayerLeftGame(class AXBlasterPlayerState* LeavingPlayerState);

public:
	//设置从DelayedStart中需要等待的时间，然后调用StartMatch()
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;

	//进入关卡的时间
	float LevelStartingTime = 0.f;

	//依据关卡的时间
	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.0f;

	//CoolDown Time
	UPROPERTY(EditDefaultsOnly)
		float CoolDownTime = 10.0f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	//倒计时时间，当和WarmUpTime相等StartMatch()
	float CountDownTime = 0.f;
public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
