// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XBlasterGameMode.generated.h"

//����MatchState
namespace MatchState
{
	extern XBLASTER_CP_API const FName CoolDown;//��Ϸ��������ʾ��ʤ��ң����ҿ�����ʾ��������ʱ��
}

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:

	//�����ӳ��������ù��캯��
	AXBlasterGameMode();

	virtual void Tick(float DeltatTime) override;

	//��ɫ����
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController);
	//������ɫ
	virtual void RequestRespawn(class AXCharacter* ElimmedCharacter, AController* ElimmedController);
	//��ɫ�˳���Ϸ
	void PlayerLeftGame(class AXBlasterPlayerState* LeavingPlayerState);

public:
	//���ô�DelayedStart����Ҫ�ȴ���ʱ�䣬Ȼ�����StartMatch()
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;

	//����ؿ���ʱ��
	float LevelStartingTime = 0.f;

	//���ݹؿ���ʱ��
	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.0f;

	//CoolDown Time
	UPROPERTY(EditDefaultsOnly)
		float CoolDownTime = 10.0f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	//����ʱʱ�䣬����WarmUpTime���StartMatch()
	float CountDownTime = 0.f;
public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
