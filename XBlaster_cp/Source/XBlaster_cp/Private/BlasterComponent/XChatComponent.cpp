// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/XChatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/XBlasterHUD.h"
#include "ChatSystem/Widget/XChatWidget.h"
#include "../XTypeHeadFile/XChatEnum.h"
#include "ChatSystem/Widget/XTextFieldWidget.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "MultiplayerSessionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "ChatSystem/Widget/XTextWidget.h"
#include "PlayerController/XBlasterPlayerController.h"

// Sets default values for this component's properties
UXChatComponent::UXChatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UXChatComponent::BeginPlay()
{
	Super::BeginPlay();
	SetSendName();
	//if (bShowJoinMessage)
	//{
	//	JoinMessage();
	//}
	// ...

}


// Called every frame
void UXChatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXChatComponent::ChatMessage_Implementation(const FText& Message, EMessageTypes Type, const FString& PlayerName, FLinearColor Color, EChatTypes ChatType)
{
	MultiSendMessage(Message, Type, PlayerName, Color, ChatType);
}

void UXChatComponent::SetSendName()
{
	UMultiplayerSessionSubsystem* GameInstance_GetName = Cast<UMultiplayerSessionSubsystem>(GetWorld()->GetGameInstance());
	SendName = GameInstance_GetName->GetSteamName();
}

void UXChatComponent::MultiExcuteCommand_Implementation(const FString& CommandId, ECommandTypes CPT)
{
	ServerSetName(CommandId);
}

void UXChatComponent::ToggleChatWindow()
{
	bToggleChat = !bToggleChat;
	AXBlasterPlayerController* PIC = Cast<AXBlasterPlayerController>(GetOwner());
	PIC->SetShowMouseCursor(bToggleChat);
	if (!bToggleChat)
	{
		PIC->SetInputMode(FInputModeGameOnly());
	}
	else
	{
		PIC->SetInputMode(FInputModeGameAndUI());
	}
	AXBlasterHUD* HUD = Cast<AXBlasterHUD>(PIC->GetHUD());
	HUD->ChatWdg->ToggleChatBase(bToggleChat);
	if (bToggleChat)
	{
		HUD->ChatWdg->TextField->SendText->SetKeyboardFocus();
	}
}

void UXChatComponent::JoinMessage()
{
	if (!bShowJoinMessage) return;
	EMessageTypes ColorType = EMessageTypes::EMT_Server;
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AXBlasterPlayerController::StaticClass(), Actors);
	TArray<APlayerController*> Controllers;
	for (auto& it : Actors)
	{
		UActorComponent* cont = it->GetComponentByClass(UXChatComponent::StaticClass());
		if (cont && cont->Implements<UXChatInterface>())
		{
			IXChatInterface::Execute_ChatMessage(
				cont,
				JoinMessageText,
				EMessageTypes::EMT_Server,
				SendName,
				SetMessageTypeColor(ColorType),
				EChatTypes::ECT_Server
			);
		}
	}
}

FLinearColor UXChatComponent::SetMessageTypeColor(EMessageTypes& MessageType)
{
	switch (MessageType)
	{
	case EMessageTypes::EMT_NormalMessage:
		return FColor::White;
		break;
	case EMessageTypes::EMT_Server:
		return FColor::Yellow;
		break;
	case EMessageTypes::EMT_Error:
		return FColor::Red;
		break;
	case EMessageTypes::EMT_Warning:
		return FColor::Orange;
		break;
	case EMessageTypes::EMT_Success:
		return FColor::Green;
		break;
	case EMessageTypes::EMT_DirectMessage:
		return FColor::Purple;
		break;
	default:
		break;
	}
	return FColor::White;
}

void UXChatComponent::ServerEnterText_Implementation(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, EChatTypes ChatType)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		EnterText(Message, MessageType, PlayerName, ChatType);
		UE_LOG(LogTemp, Warning, TEXT("ServerEnterText"));
	}
}

void UXChatComponent::ServerSetName_Implementation(const FString& Name)
{
	//»ñÈ¡SteamÃû×Ö
	UMultiplayerSessionSubsystem* GameInstance_GetName = Cast<UMultiplayerSessionSubsystem>(GetWorld()->GetGameInstance());
	SendName = GameInstance_GetName->GetSteamName();
}

void UXChatComponent::EnterText(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, EChatTypes ChatType)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AXBlasterPlayerController::StaticClass(), Actors);
	for (auto& it : Actors)
	{
		UActorComponent* cont = it->GetComponentByClass(UXChatComponent::StaticClass());
		if (cont && cont->Implements<UXChatInterface>())
		{
			IXChatInterface::Execute_ChatMessage(
				cont,
				Message,
				MessageType,
				PlayerName,
				SetMessageTypeColor(MessageType),
				ChatType
			);
		}
	}
}

void UXChatComponent::SendMessage(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, EChatTypes ChatType)
{
	AXBlasterPlayerController* PIC = Cast<AXBlasterPlayerController>(GetOwner());

	if (!PIC->IsLocalController()) return;
	AXBlasterHUD* HUD = Cast<AXBlasterHUD>(PIC->GetHUD());
	if (HUD)
	{
		if (HUD->ChatWdg)
		{
			UXTextWidget* TextWidget = CreateWidget<UXTextWidget>(
				UGameplayStatics::GetPlayerController(GetWorld(), 0),
				HUD->ChatTextClass
			);
			if (TextWidget)
			{
				TextWidget->PlayerName = FText::FromString(SendName);
				TextWidget->InText = Message;
				TextWidget->MessType = MessageType;
				TextWidget->ChatType = EChatTypes::ECT_All;
				HUD->ChatWdg->ChatScrollBox->AddChild(TextWidget);
				HUD->ChatWdg->ChatScrollBox->ScrollToEnd();
			}
		}
	}
}

void UXChatComponent::MultiSendMessage_Implementation(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, EChatTypes ChatType)
{
	SendMessage(Message, MessageType, PlayerName, color, ChatType);
}

