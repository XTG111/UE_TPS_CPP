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

protected:
	virtual void BeginPlay() override;

private:

	//������ҿ�����������HUD
	class AXBlasterHUD* XBlasterHUD;
	
};
