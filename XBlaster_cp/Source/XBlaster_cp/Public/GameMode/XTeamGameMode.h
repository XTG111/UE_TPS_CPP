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
	//��;��������ȷ�����黮��
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//����뿪ʱ�߳�����
	virtual void Logout(AController* Exiting) override;
	//��������
	virtual float CalculateDamage(AController* Attacker, AController* Vicitim, float BaseDamage) override;
	//�������
	virtual void PlayerEliminated(class AXCharacter* ElimmedCharacter, class AXBlasterPlayerController* VictimController, class AXBlasterPlayerController* AttackerController) override;

protected:
	//��Ϸ��ʼʱ��������ȷ�����黮��
	virtual void HandleMatchHasStarted() override;

private:
	UPROPERTY(EditAnywhere)
		bool bCanFireFriend = false;
	
};
