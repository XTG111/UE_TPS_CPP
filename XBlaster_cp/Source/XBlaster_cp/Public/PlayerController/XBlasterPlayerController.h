// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XBlasterPlayerController.generated.h"

//广播Ping
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 *
 */
UCLASS()
class XBLASTER_CP_API AXBlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AXBlasterPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//synced with server clock;计算当前控制器控制的客户端对应的服务器准确时刻
	virtual float GetSeverTime();
	//利用该函数使得同步时间可以最快
	virtual void ReceivedPlayer() override;

	//设置Health
	void SetHUDHealth(float Health, float MaxHealth);
	//设置Shield
	void SetHUDShield(float Shield, float MaxShield);

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
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	//设置手榴弹数
	void SetHUDGrenadeAmount(int32 GrenadeAmount);

	//客户端到服务器的时间
	UPROPERTY()
		float SingleTripTime = 0.f;

	//用来广播Ping过高
	FHighPingDelegate HighPingDelegate;

	//击杀文本广播
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	//设置团队得分UI
	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScores(int32 RedTeamScore);
	void SetHUDBlueTeamScores(int32 BlueTeamScore);

	UFUNCTION(Server, Reliable)
		void ServerReturnToMainMenu();
	UFUNCTION(Client, Reliable)
		void ClientReturnToMenu();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	//绑定退出按键
	virtual void SetupInputComponent() override;

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
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();
	//当客户端加入时的状态，因为所有客户端相对于服务器来说都是在游戏中加入的
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown);

	/**
	处理MatchState的不同状态下的操作
	*/
	//InProgress
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	//CoolDown
	void HandleCoolDown();

	//播放UI动画HighPing
	void HighPingWarning();
	void StopHighPingWarning();

	//tick控制highping的播放
	void PlayHighPingAnim(float DeltaTime);

	//显示退出界面的UI
	void ShowReturnToMainMenu();

	//Client RPC
	UFUNCTION(Client, Reliable)
		void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	//控制客户端上的分数显示
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamsScores)
		bool bShowTeamScores = false;
	UFUNCTION()
		void OnRep_ShowTeamsScores();

	//用于显示文本
	FString GetInfoText(const TArray<class AXBlasterPlayerState*>& Players);
	//用于团队游戏显示文本
	FString GetTeamInfoText(const class AXBlasterGameState* XBlasterGameState);

private:

	//利用玩家控制器来设置HUD
	UPROPERTY()
		class AXBlasterHUD* XBlasterHUD;

	//获取GameMode
	UPROPERTY()
		class AXBlasterGameMode* XBlasterGameMode;

	//关卡启动时的世界时间
	float LevelStartingTime = 0.f;

	//比赛的总时间设置SetHUDGameTime
	float MatchTime = 0.f;
	uint32 CountDownInt = 0;
	//比赛热身时间
	float WarmupTime = 0.f;
	//比赛结束时间冷却
	float CoolDownTime = 0.f;

	//设置当前游戏地图状态MatchState;
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;
	UFUNCTION()
		void OnRep_MatchState();

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWdg;


	//通过添加布尔值，解决HUD的绘制，血条这些的绘制是需要有actor之后才能够绘制，但我们修改了绘制UI的调用，导致我们绘制UI时这些属性还没有
	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenade = false;
	bool bInitializeShield = false;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeCarriedAmmo = false;

	float HUDHealth;
	float HUDScore;
	float HUDMaxHealth;
	int32 HUDDefeats;
	int32 HUDGrenadeAmount;
	float HUDShield;
	float HUDMaxShield;
	float HUDWeaponAmmo;
	float HUDCarriedAmmo;

	/*ping时间*/
	//上次显示时间
	float HighPingRunningTime = 0.f;
	//动画播放总时长
	UPROPERTY(EditAnywhere)
		float HighPingDuration = 5.f;
	//动画播放的时间
	float PingAnimationRunningTime = 0.f;
	//检测ping的间隔,以及pingwarning冷却时间
	UPROPERTY(EditAnywhere)
		float CheckPingFrequency = 20.f;
	//ServerRPC,用来通知服务器此时的Ping状态 以更改serverrewind
	UFUNCTION(Server, Reliable)
		void ServerReportPingStatus(bool bHighPing);

	//ping的阈值
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 50.f;

	/*退出界面的控制*/
	UPROPERTY(EditAnywhere, Category = HUD)
		TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	//退出界面UI的实例
	UPROPERTY()
		class UReturnToMainMenuWidget* ReturnToMainMenu;

	//用来确定是否打开了这个UI
	bool bReturnToMainMenuOpen = false;

	//Chat System
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UXChatComponent* ChatComponent;
	void BeginChat();
};
