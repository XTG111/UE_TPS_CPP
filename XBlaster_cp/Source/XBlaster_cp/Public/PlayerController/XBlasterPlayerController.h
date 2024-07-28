// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XBlasterPlayerController.generated.h"

//�㲥Ping
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

	//synced with server clock;���㵱ǰ���������ƵĿͻ��˶�Ӧ�ķ�����׼ȷʱ��
	virtual float GetSeverTime();
	//���øú���ʹ��ͬ��ʱ��������
	virtual void ReceivedPlayer() override;

	//����Health
	void SetHUDHealth(float Health, float MaxHealth);
	//����Shield
	void SetHUDShield(float Shield, float MaxShield);

	void OnPossess(APawn* InPawn) override;

	//��ɱ��
	void SetHUDScore(float Score);
	//������
	void SetHUDDefeats(int32 Defeats);
	//�ӵ�����
	void SetHUDWeaponAmmo(int32 Ammo);
	//������
	void SetHUDCarriedAmmo(int32 Ammo);
	//���Ʊ���ʣ��ʱ��
	void SetHUDGameTime(float CountDownTime);
	//��������ʣ��ʱ��
	void SetHUDAnnouncementCountDown(float CountDownTime);
	//���õ�ǰ����״̬
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	//����������
	void SetHUDGrenadeAmount(int32 GrenadeAmount);

	//�ͻ��˵���������ʱ��
	UPROPERTY()
		float SingleTripTime = 0.f;

	//�����㲥Ping����
	FHighPingDelegate HighPingDelegate;

	//��ɱ�ı��㲥
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);

	//�����Ŷӵ÷�UI
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

	//���˳�����
	virtual void SetupInputComponent() override;

	//����������Ҫ��ʱ���������ʱ�����Ϸʱ�䣬������ʵ�ʵĻ��ƺ���
	void SetHUDTime();

	//�������Ϳͻ��˵�ʱ��ͬ��

	//���Ϳͻ��˵�ǰʱ�̵������������������ʱ��
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequese);

	//���������͸��ͻ���:�ͻ��˷�����һ��ServerRequestServerTimeRPC��ʱ�̲����ط�������Ӧ���RPC��ʱ��
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//Current Server AND Client Time Delta
	float ClientServerDelta = 0.f;

	//����Ϸ������ÿ��һ��ʱ��ͬ��������ʱ�䵽�ͻ���
	UPROPERTY(EditAnywhere, Category = Time)
		float TimeSyncFrequency = 3.f;
	//��¼��һ��ͬ���Ĺ�ȥʱ��
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	//�ֶ���ʼ��CharacterOverlayUI
	void PollInit();

	//RPC��⵱ǰ��������Ϸ״̬
	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();
	//���ͻ��˼���ʱ��״̬����Ϊ���пͻ�������ڷ�������˵��������Ϸ�м����
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown);

	/**
	����MatchState�Ĳ�ͬ״̬�µĲ���
	*/
	//InProgress
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	//CoolDown
	void HandleCoolDown();

	//����UI����HighPing
	void HighPingWarning();
	void StopHighPingWarning();

	//tick����highping�Ĳ���
	void PlayHighPingAnim(float DeltaTime);

	//��ʾ�˳������UI
	void ShowReturnToMainMenu();

	//Client RPC
	UFUNCTION(Client, Reliable)
		void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	//���ƿͻ����ϵķ�����ʾ
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamsScores)
		bool bShowTeamScores = false;
	UFUNCTION()
		void OnRep_ShowTeamsScores();

	//������ʾ�ı�
	FString GetInfoText(const TArray<class AXBlasterPlayerState*>& Players);
	//�����Ŷ���Ϸ��ʾ�ı�
	FString GetTeamInfoText(const class AXBlasterGameState* XBlasterGameState);

private:

	//������ҿ�����������HUD
	UPROPERTY()
		class AXBlasterHUD* XBlasterHUD;

	//��ȡGameMode
	UPROPERTY()
		class AXBlasterGameMode* XBlasterGameMode;

	//�ؿ�����ʱ������ʱ��
	float LevelStartingTime = 0.f;

	//��������ʱ������SetHUDGameTime
	float MatchTime = 0.f;
	uint32 CountDownInt = 0;
	//��������ʱ��
	float WarmupTime = 0.f;
	//��������ʱ����ȴ
	float CoolDownTime = 0.f;

	//���õ�ǰ��Ϸ��ͼ״̬MatchState;
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;
	UFUNCTION()
		void OnRep_MatchState();

	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWdg;


	//ͨ����Ӳ���ֵ�����HUD�Ļ��ƣ�Ѫ����Щ�Ļ�������Ҫ��actor֮����ܹ����ƣ��������޸��˻���UI�ĵ��ã��������ǻ���UIʱ��Щ���Ի�û��
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

	/*pingʱ��*/
	//�ϴ���ʾʱ��
	float HighPingRunningTime = 0.f;
	//����������ʱ��
	UPROPERTY(EditAnywhere)
		float HighPingDuration = 5.f;
	//�������ŵ�ʱ��
	float PingAnimationRunningTime = 0.f;
	//���ping�ļ��,�Լ�pingwarning��ȴʱ��
	UPROPERTY(EditAnywhere)
		float CheckPingFrequency = 20.f;
	//ServerRPC,����֪ͨ��������ʱ��Ping״̬ �Ը���serverrewind
	UFUNCTION(Server, Reliable)
		void ServerReportPingStatus(bool bHighPing);

	//ping����ֵ
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 50.f;

	/*�˳�����Ŀ���*/
	UPROPERTY(EditAnywhere, Category = HUD)
		TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	//�˳�����UI��ʵ��
	UPROPERTY()
		class UReturnToMainMenuWidget* ReturnToMainMenu;

	//����ȷ���Ƿ�������UI
	bool bReturnToMainMenuOpen = false;

	//Chat System
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UXChatComponent* ChatComponent;
	void BeginChat();
};
