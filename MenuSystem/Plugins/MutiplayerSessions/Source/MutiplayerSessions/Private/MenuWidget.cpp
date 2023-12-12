// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "MultiplayerSessionSubsystem.h"
#include "Components/Button.h"

void UMenuWidget::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch)
{
	//ʹ�ò��������β�ֵ
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	//��ӵ��ӿ�
	AddToViewport();

	//���ÿɼ���
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	//������ܿ���ΪUIģʽ�����������ɫ
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			//���þ۽�����Ϊ��ǰ��UI����TakeWidget���
			InputModeData.SetWidgetToFocus(TakeWidget());
			//�Ƿ����������UI����
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);

			//��ʾ���
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//ʵ������ϵͳ����
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}
}

bool UMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	//�����Դ�ί�а󶨻ص�����
	if (Button_Host)
	{
		Button_Host->OnClicked.AddDynamic(this, &UMenuWidget::Button_HostClicked);
	}
	if (Button_Join)
	{
		Button_Join->OnClicked.AddDynamic(this, &UMenuWidget::Button_JoinClicked);
	}
	return true;
}

void UMenuWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	//��ת�����ؿ���ɾ����ʼUI
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenuWidget::Button_HostClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Host Button Clicked")));
	}

	//����Subsytem�е�ʵ�ʺ���
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnections, MatchType);
		//UWorld* World = GetWorld();
		//if (World)
		//{
		//	World->ServerTravel("/Game/ThirdPersonCPP/Maps/Lobby?listen");
		//}
	}
	
}

void UMenuWidget::Button_JoinClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Join Button Clicked")));
	}
	if (MultiplayerSessionSubsystem)
	{
		//MultiplayerSessionSubsystem->JoinSession();
	}
}

void UMenuWidget::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
