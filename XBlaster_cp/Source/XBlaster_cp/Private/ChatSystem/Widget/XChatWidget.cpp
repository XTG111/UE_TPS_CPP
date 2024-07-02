// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/Widget/XChatWidget.h"
#include "ChatSystem/Widget/XTextFieldWidget.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "BlasterComponent/XChatComponent.h"
#include "Components/ScrollBox.h"


void UXChatWidget::NativeConstruct()
{
	TextField->OnSendText.AddDynamic(this, &UXChatWidget::OnSendText);
}

void UXChatWidget::ToggleChatBase(bool btoggle)
{
	if (!TextField) return;
	TextField->ToggleChat(btoggle);
	EUMGSequencePlayMode::Type pmode = btoggle ? EUMGSequencePlayMode::Reverse : EUMGSequencePlayMode::Forward;
	PlayAnimation(Toggle, 0.0f, 1, pmode, 1.0f, false);
	OnToggleChat.Broadcast(btoggle);
}

void UXChatWidget::OnSendText(const FText& text)
{
	UE_LOG(LogTemp, Warning, TEXT("OnSendText"));
	ECommandTypes CTP = ECommandTypes::ECoT_SetName;
	FString Arguments;
	bool bValid = RefindCommands(text, CTP, Arguments);
	if (bValid)
	{
		GetNowUserComponent(GetOwningPlayer())->MultiExcuteCommand(Arguments, CTP);
	}
	else
	{
		if (UKismetTextLibrary::NotEqual_TextText(text, FText::FromString(TEXT(""))))
		{
			GetNowUserComponent(GetOwningPlayer())->ServerEnterText(
				text,
				EMessageTypes::EMT_NormalMessage,
				GetNowUserComponent(GetOwningPlayer())->SendName,
				ChatType
			);
		}

	}
}

bool UXChatWidget::RefindCommands(const FText& InText, ECommandTypes CTP, FString& Arguments)
{
	FString EnumName = "ECommandTypes";
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	TArray<FString> Commands;
	Commands.Add(EnumPtr->GetNameStringByIndex(static_cast<int32>(ECommandTypes::ECoT_PrintHello)).ToLower());
	Commands.Add(EnumPtr->GetNameStringByIndex(static_cast<int32>(ECommandTypes::ECoT_SetName)).ToLower());

	FString CommandLines;
	FString CommandString;

	RebuildCommands(InText, CommandString, CommandLines);

	if (Commands.Contains(CommandLines))
	{
		int64 cuurentEnumValue = EnumPtr->GetValueByNameString(CommandLines);
		bool isEnumValid = EnumPtr->IsValidEnumValue(cuurentEnumValue);
		if (isEnumValid)
		{
			CTP = ECommandTypes(cuurentEnumValue);
		}
		Arguments = UKismetStringLibrary::Trim(UKismetStringLibrary::Replace(
			InText.ToString(),
			CommandString,
			FString(""),
			ESearchCase::IgnoreCase
		));
		return true;
	}
	else
	{
		CTP = ECommandTypes::ECoT_SetName;
		Arguments = "";
	}
	return false;
}

void UXChatWidget::RebuildCommands(const FText& InText, FString& Output, FString& res)
{
	FString temp = InText.ToString();
	TArray<FString> TextParse = UKismetStringLibrary::ParseIntoArray(temp);
	FString to = "";
	if (TextParse.Num() != 0)
	{
		Output = TextParse[ReaserchLen];
		res = UKismetStringLibrary::ToLower(UKismetStringLibrary::Replace(
			TextParse[ReaserchLen],
			GetNowUserComponent(GetOwningPlayer())->CommandsPrefix,
			to,
			ESearchCase::IgnoreCase
		));
	}
}

UXChatComponent* UXChatWidget::GetNowUserComponent(AActor* Owner)
{
	UXChatComponent* res = Cast<UXChatComponent>(Owner->GetComponentByClass(UXChatComponent::StaticClass()));
	if (res) return res;
	else return nullptr;
}


