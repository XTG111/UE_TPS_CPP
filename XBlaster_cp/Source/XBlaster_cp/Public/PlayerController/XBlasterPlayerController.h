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
	//����ʣ���¼�
	void SetHUDGameTime(float CountDownTime);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//����ʣ��ʱ��
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

private:

	//������ҿ�����������HUD
	UPROPERTY()
	class AXBlasterHUD* XBlasterHUD;

	//��������ʱ������SetHUDGameTime
	float MatchTime = 120.f;
	uint32 CountDownInt = 0;
	
};
