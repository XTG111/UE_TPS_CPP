// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"

void UMenuWidget::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch)
{
	//使得参数接受形参值
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	//添加到视口
	AddToViewport();

	//设置可见性
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	//输入接受控制为UI模式，避免操作角色
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			//设置聚焦对象，为当前的UI利用TakeWidget获得
			InputModeData.SetWidgetToFocus(TakeWidget());
			//是否锁定鼠标在UI界面
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);

			//显示鼠标
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//实例化子系统变量
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	//绑定回调函数
	if (MultiplayerSessionSubsystem)
	{
		//动态委托的绑定回调函数
		MultiplayerSessionSubsystem->MultiPlayerOnCreateSessionComplete.AddDynamic(this, &UMenuWidget::OnCreateSession);
		MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.AddDynamic(this, &UMenuWidget::OnDestroySession);
		MultiplayerSessionSubsystem->MultiPlayerOnStartSessionComplete.AddDynamic(this, &UMenuWidget::OnStartSession);

		//静态委托绑定回调函数
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

	//利用自带委托绑定回调函数
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
	//跳转大厅关卡后，删除初始UI
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}


//自定义委托的回调函数
void UMenuWidget::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Session Created Successfully")));
		}
		//当会话创建成功才进行跳转
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

//接受SessionInterface传回给SessIonSubsystem的结果，由SessionSubsytem利用委托传过来，然后进行处理寻找
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
	//为了使得菜单与插件相互独立，所以我们需要单独获取SessionInterface
	//获得onlinesubsystem的变量
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		//将会话接口实例化
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Addr;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Addr);

			//客户端利用ClientTrave进行地图转换
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

	//调用Subsytem中的实际函数
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
