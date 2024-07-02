// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/Widget/XTextFieldWidget.h"
#include "Components/EditableText.h"
#include "BlasterComponent/XChatComponent.h"
#include "Kismet/GameplayStatics.h"


void UXTextFieldWidget::NativeConstruct()
{
	if (SendText)
	{
		SendText->OnTextCommitted.AddDynamic(this, &UXTextFieldWidget::TextCommitted);
	}
}

//¿ØÖÆ¶¯»­²¥·Å
void UXTextFieldWidget::ToggleChat(bool btoggle)
{
	if (Toggle)
	{
		EUMGSequencePlayMode::Type pmode = btoggle ? EUMGSequencePlayMode::Reverse : EUMGSequencePlayMode::Forward;
		PlayAnimation(Toggle, 0.0f, 1, pmode, 1.0f, false);
	}
}

void UXTextFieldWidget::TextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	switch (CommitMethod)
	{
	case ETextCommit::Default:
		break;
	case ETextCommit::OnEnter:
		UE_LOG(LogTemp, Warning, TEXT("TextCommitted_OnEnter"));
		OnSendText.Broadcast(Text);
		SendText->SetText(FText::FromString(""));
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeGameOnly());
		GetNowUserComponent(GetOwningPlayer())->ToggleChatWindow();
		break;
	case ETextCommit::OnUserMovedFocus:
		break;
	case ETextCommit::OnCleared:
		break;
	default:
		break;
	}
}

UXChatComponent* UXTextFieldWidget::GetNowUserComponent(AActor* Owner)
{
	UXChatComponent* res = Cast<UXChatComponent>(Owner->GetComponentByClass(UXChatComponent::StaticClass()));
	if (res) return res;
	else return nullptr;
}
