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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	//绘制比赛剩余时间
	void SetHUDGameTime(float CountDownTime);
	//绘制热身剩余时间
	void SetHUDAnnouncementCountDown(float CountDownTime);
	//设置当前比赛状态
	void OnMatchStateSet(FName State);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//计算所有需要的时间比如热身时间和游戏时间，并调用实际的绘制函数
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
	//手动初始化CharacterOverlayUI
	void PollInit();
	
	//RPC检测当前服务器游戏状态
	UFUNCTION(Server,Reliable)
		void ServerCheckMatchState();
	//当客户端加入时的状态，因为所有客户端相对于服务器来说都是在游戏中加入的
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime);

	/**
	处理MatchState的不同状态下的操作
	*/
	//InProgress
	void HandleMatchHasStarted();
	//CoolDown
	void HandleCoolDown();

private:

	//利用玩家控制器来设置HUD
	UPROPERTY()
		class AXBlasterHUD* XBlasterHUD;

	//关卡启动时的世界时间
	float LevelStartingTime = 0.f;

	//比赛的总时间设置SetHUDGameTime
	float MatchTime = 0.f;
	uint32 CountDownInt = 0;

	//比赛热身时间
	float WarmupTime = 0.f;

	//设置当前游戏地图状态MatchState;
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;
	UFUNCTION()
		void OnRep_MatchState();
	
	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWdg;


	//通过添加布尔值，解决HUD的绘制，血条这些的绘制是需要有actor之后才能够绘制，但我们修改了绘制UI的调用，导致我们绘制UI时这些属性还没有
	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDScore;
	float HUDMaxHealth;
	int32 HUDDefeats;
};
