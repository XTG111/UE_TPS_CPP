// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UReturnToMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//����˵�
	void MenuSet();
	//���ٲ˵�
	void MenuTearDown();

protected:
	virtual bool Initialize() override;

	//���ð󶨵���ϵͳ�е�����ί��
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);

	//��ɫ�뿪��Ϸʱ��ί�лص�����
	UFUNCTION()
		void OnPlayerLeftGame();

private:
	UPROPERTY()
		class APlayerController* PlayerController;
	UPROPERTY(meta = (BindWidget))
		class UButton* Return2MainMenuButton;
	//�ص�����
	UFUNCTION()
		void Return2MainMenuButtonClicked();
	//�Ự����
	UPROPERTY()
		class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;
};
