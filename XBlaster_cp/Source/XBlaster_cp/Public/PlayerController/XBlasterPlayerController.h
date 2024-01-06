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

	//synced with server clock;���㵱ǰ���������ƵĿͻ��˶�Ӧ�ķ�����׼ȷʱ��
	virtual float GetSeverTime();
	//���øú���ʹ��ͬ��ʱ��������
	virtual void ReceivedPlayer() override;

	//����Health
	void SetHUDHealth(float Health, float MaxHealth);

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
	void OnMatchStateSet(FName State);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
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
	UFUNCTION(Server,Reliable)
		void ServerCheckMatchState();
	//���ͻ��˼���ʱ��״̬����Ϊ���пͻ�������ڷ�������˵��������Ϸ�м����
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime);

	/**
	����MatchState�Ĳ�ͬ״̬�µĲ���
	*/
	//InProgress
	void HandleMatchHasStarted();
	//CoolDown
	void HandleCoolDown();

private:

	//������ҿ�����������HUD
	UPROPERTY()
		class AXBlasterHUD* XBlasterHUD;

	//�ؿ�����ʱ������ʱ��
	float LevelStartingTime = 0.f;

	//��������ʱ������SetHUDGameTime
	float MatchTime = 0.f;
	uint32 CountDownInt = 0;

	//��������ʱ��
	float WarmupTime = 0.f;

	//���õ�ǰ��Ϸ��ͼ״̬MatchState;
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;
	UFUNCTION()
		void OnRep_MatchState();
	
	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWdg;


	//ͨ����Ӳ���ֵ�����HUD�Ļ��ƣ�Ѫ����Щ�Ļ�������Ҫ��actor֮����ܹ����ƣ��������޸��˻���UI�ĵ��ã��������ǻ���UIʱ��Щ���Ի�û��
	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDScore;
	float HUDMaxHealth;
	int32 HUDDefeats;
};
