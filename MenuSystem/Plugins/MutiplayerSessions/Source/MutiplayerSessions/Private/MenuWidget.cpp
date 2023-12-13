// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"

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

	//�󶨻ص�����
	if (MultiplayerSessionSubsystem)
	{
		//��̬ί�еİ󶨻ص�����
		MultiplayerSessionSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &UMenuWidget::OnCreateSession);
		MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &UMenuWidget::OnDestroySession);
		MultiplayerSessionSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this, &UMenuWidget::OnStartSession);

		//��̬ί�а󶨻ص�����
		MultiplayerSessionSubsystem->MultiPlayerOnFindSessionComplete.AddUObject(this,&UMenuWidget::OnFindSession);
		MultiplayerSessionSubsystem->MultiPlayerOnJoinSessionComplete.AddUObject(this, &UMenuWidget::OnJoinSession);

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


//�Զ���ί�еĻص�����
void UMenuWidget::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Session Created Successfully")));
		}
		//���Ự�����ɹ��Ž�����ת
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel("/Game/ThirdPersonCPP/Maps/Lobby?listen");
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString(TEXT("Session Created Failed")));
		}
	}
	
}

void UMenuWidget::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuWidget::OnStartSession(bool bWasSuccessful)
{
}

//����SessionInterface���ظ�SessIonSubsystem�Ľ������SessionSubsytem����ί�д�������Ȼ����д���Ѱ��
void UMenuWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchTypeXTG111"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			if (MultiplayerSessionSubsystem)
			{
				MultiplayerSessionSubsystem->JoinSession(Result);
				return;
			}
		}
	}
}

void UMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	//Ϊ��ʹ�ò˵������໥����������������Ҫ������ȡSessionInterface
	//���onlinesubsystem�ı���
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		//���Ự�ӿ�ʵ����
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Addr;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Addr);

			//�ͻ�������ClientTrave���е�ͼת��
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Addr, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMenuWidget::Button_HostClicked()
{
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Host Button Clicked")));
	//}

	//����Subsytem�е�ʵ�ʺ���
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
	
}

void UMenuWidget::Button_JoinClicked()
{
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Join Button Clicked")));
	//}
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->FindSessions(50000);
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
