// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "MultiplayerSessionSubsystem.h"
#include "Components/Button.h"

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

void UMenuWidget::Button_HostClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Host Button Clicked")));
	}

	//调用Subsytem中的实际函数
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
