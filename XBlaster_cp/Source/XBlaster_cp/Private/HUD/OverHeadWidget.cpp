// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverHeadWidget.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionSubsystem.h"

void UOverHeadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
	RemoveFromParent();
}

void UOverHeadWidget::SetDisplayeText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//ENetRole LocalRole = InPawn->GetLocalRole();
	//FString Role;
	FString ActorName;
	//获取当前玩家名字
	if (InPawn) {
		ActorName = InPawn->GetName();
		UMultiplayerSessionSubsystem* GameInstance_GetName = InPawn->GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();
		if (GameInstance_GetName)
		{
			ActorName = GameInstance_GetName->GetSteamName();
			//ActorName = FString(TEXT("Player"));
		}
		//if (InPawn->GetGameInstance()) ActorName = FString(TEXT("NiHao"));
	}
	SetDisplayeText(ActorName);
	////获取当前玩家所有权
	//switch ( LocalRole)
	//{
	//case ENetRole::ROLE_Authority:
	//	Role = FString("Authority");
	//	break;
	//case ENetRole::ROLE_AutonomousProxy:
	//	Role = FString("AutonomousProxy");
	//	break;
	//case ENetRole::ROLE_SimulatedProxy:
	//	Role = FString("SimulatedProxy");
	//	break;
	//case ENetRole::ROLE_None:
	//	Role = FString("None");
	//	break;
	//}
	//FString LocalRoleString = FString::Printf(TEXT("%s Local Role:%s"), *ActorName, *Role);
	//SetDisplayeText(LocalRoleString);

	////RemoteRole
	//ENetRole RemoteRole = InPawn->GetRemoteRole();
	//FString Role;
	//FString ActorName;
	//////获取当前玩家名字
	//if (InPawn) {
	//	ActorName = InPawn->GetName();
	//}

	//////获取当前玩家所有权
	//switch (RemoteRole)
	//{
	//case ENetRole::ROLE_Authority:
	//	Role = FString("Authority");
	//	break;
	//case ENetRole::ROLE_AutonomousProxy:
	//	Role = FString("AutonomousProxy");
	//	break;
	//case ENetRole::ROLE_SimulatedProxy:
	//	Role = FString("SimulatedProxy");
	//	break;
	//case ENetRole::ROLE_None:
	//	Role = FString("None");
	//	break;
	//}
	//FString RemoteRoleString = FString::Printf(TEXT("%s Local Role:%s"), *ActorName, *Role);
	//SetDisplayeText(RemoteRoleString);
}
