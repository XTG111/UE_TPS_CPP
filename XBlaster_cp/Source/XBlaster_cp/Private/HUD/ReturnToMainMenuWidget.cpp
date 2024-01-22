// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ReturnToMainMenuWidget.h"
#include "Components/Button.h"
#include "MutiplayerSessions/Public/MultiplayerSessionSubsystem.h"
#include "GameFrameWork/GameModeBase.h"
#include "Character/XCharacter.h"

void UReturnToMainMenuWidget::MenuSet()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//�󶨵���ص�����
	if (Return2MainMenuButton && !Return2MainMenuButton->OnClicked.IsBound())
	{
		Return2MainMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenuWidget::Return2MainMenuButtonClicked);
	}

	//��ȡ��������ϵͳ ͨ��GameInstance
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		if (MultiplayerSessionSubsystem && !MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.IsBound())
		{
			//�����е�����ί�н��а�
			MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenuWidget::OnDestroySession);
		}
	}
}

bool UReturnToMainMenuWidget::Initialize()
{
	
	if (!Super::Initialize())
	{
		return false;
	}


	return true;
}

void UReturnToMainMenuWidget::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		Return2MainMenuButton->SetIsEnabled(true);
		return;
	}

	//�������ٻỰ��Ҫ��������
	UWorld* World = GetWorld();
	if (World)
	{
		//��ȡ��ǰ��GameMode ֻ�з������ϲ��У����Կ��������ж��Ƿ��Ƿ�����
		AGameModeBase* GameMode =  World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			//�ڿͻ�����ͨ��PlayerController�����Ʒ���
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}

	}
}

//ɾ��Widget
void UReturnToMainMenuWidget::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	//ȡ������ص������İ�
	if (Return2MainMenuButton && Return2MainMenuButton->OnClicked.IsBound())
	{
		Return2MainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenuWidget::Return2MainMenuButtonClicked);
	}
	if (MultiplayerSessionSubsystem && MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.IsBound())
	{
		//�����е�����ί�н��а�
		MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenuWidget::OnDestroySession);
	}

}

void UReturnToMainMenuWidget::Return2MainMenuButtonClicked()
{
	//����˳�����ã����û�гɹ�������ô��������
	Return2MainMenuButton->SetIsEnabled(false);

	//ͨ����ҿ����������ң��������ʱ��˵���������������˳�
	//���û�У�˵������������������������������Ҫ�������ð�ťΪ���԰���
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			AXCharacter* XCharacter = Cast<AXCharacter>(FirstPlayerController->GetPawn());
			if (XCharacter)
			{
				XCharacter->ServerLeaveGame();
				//��OnLeftGameί��
				XCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenuWidget::OnPlayerLeftGame);
			}
			else
			{
				Return2MainMenuButton->SetIsEnabled(true);
			}
		}
	}
}

/*
* ��ɫ�˳���Ϸ
*/
void UReturnToMainMenuWidget::OnPlayerLeftGame()
{
	//���������ϵĻỰ
	if (MultiplayerSessionSubsystem)
	{
		//������ϵͳ�е����ٻỰ�������жϿ�����
		MultiplayerSessionSubsystem->DestroySession();
	}
}