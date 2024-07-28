// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ReturnToMainMenuWidget.h"
#include "Components/Button.h"
#include "MutiplayerSessions/Public/MultiplayerSessionSubsystem.h"
#include "GameFrameWork/GameModeBase.h"
#include "Character/XCharacter.h"
#include "PlayerController/XBlasterPlayerController.h"

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

	//绑定点击回调函数
	if (Return2MainMenuButton && !Return2MainMenuButton->OnClicked.IsBound())
	{
		Return2MainMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenuWidget::Return2MainMenuButtonClicked);
	}

	//获取创建的子系统 通过GameInstance
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		if (MultiplayerSessionSubsystem && !MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.IsBound())
		{
			//对其中的销毁委托进行绑定
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
}

//删除Widget
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
	//取消点击回调函数的绑定
	if (Return2MainMenuButton && Return2MainMenuButton->OnClicked.IsBound())
	{
		Return2MainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenuWidget::Return2MainMenuButtonClicked);
	}
	if (MultiplayerSessionSubsystem && MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.IsBound())
	{
		//对其中的销毁委托进行绑定
		MultiplayerSessionSubsystem->MultiPlayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenuWidget::OnDestroySession);
	}

}

void UReturnToMainMenuWidget::Return2MainMenuButtonClicked()
{
	//点击退出后禁用，如果没有成功消除那么重新启用
	Return2MainMenuButton->SetIsEnabled(false);

	//通过玩家控制器获得玩家，当有玩家时，说明我们正在请求退出
	//如果没有，说明我们死亡并且真正重生。所以需要重新设置按钮为可以按下
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
				//绑定OnLeftGame委托
				XCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenuWidget::OnPlayerLeftGame);
				//SetVisibility(ESlateVisibility::Hidden);
			}
			else
			{
				Return2MainMenuButton->SetIsEnabled(true);
			}
		}

		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Server Leave")));
			}
			World->ServerTravel(FString(TEXT("/Game/Maps/Level01_Map?listen")));
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Client Leave")));
				//PlayerController->ClientTravel(FString(TEXT("/Game/Maps/Level01_Map")), ETravelType::TRAVEL_Absolute);
			}
			Cast<AXBlasterPlayerController>(PlayerController)->ServerReturnToMainMenu();
		}
	}
}

/*
* 角色退出游戏
*/
void UReturnToMainMenuWidget::OnPlayerLeftGame()
{
	//销毁连接上的会话
	if (MultiplayerSessionSubsystem)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Client Leave")));
		}
		//调用子系统中的销毁会话函数进行断开连接
		MultiplayerSessionSubsystem->DestroySession();
	}
}