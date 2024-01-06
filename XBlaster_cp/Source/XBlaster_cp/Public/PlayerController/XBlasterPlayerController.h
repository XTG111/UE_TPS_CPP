// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XBlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	//synced with server clock;计算当前控制器控制的客户端对应的服务器准确时刻
	virtual float GetSeverTime();
	//利用该函数使得同步时间可以最快
	virtual void ReceivedPlayer() override;

	//设置Health
	void SetHUDHealth(float Health, float MaxHealth);

	void OnPossess(APawn* InPawn) override;

	//击杀数
	void SetHUDScore(float Score);
	//死亡数
	void SetHUDDefeats(int32 Defeats);
	//子弹数量
	void SetHUDWeaponAmmo(int32 Ammo);
	//备弹数
	void SetHUDCarriedAmmo(int32 Ammo);
	//比赛剩余事件
	void SetHUDGameTime(float CountDownTime);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//计算剩余时间
	void SetHUDTime();

	//服务器和客户端的时间同步

	//发送客户端当前时刻到服务器并请求服务器时刻
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequese);

	//服务器发送给客户端:客户端发送上一个ServerRequestServerTimeRPC的时刻并返回服务器响应这个RPC的时刻
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//Current Server AND Client Time Delta
	float ClientServerDelta = 0.f;

	//在游戏过程中每隔一段时间同步服务器时间到客户端
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 3.f;
	//记录上一次同步的过去时间
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

private:

	//利用玩家控制器来设置HUD
	UPROPERTY()
	class AXBlasterHUD* XBlasterHUD;

	//比赛的总时间设置SetHUDGameTime
	float MatchTime = 120.f;
	uint32 CountDownInt = 0;
	
};
