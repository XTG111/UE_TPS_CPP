// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatSystem/Widget/XTextWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

void UXTextWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerNameText->SetText(PlayerName);
	int hour = UKismetMathLibrary::Now().GetHour();
	int min = UKismetMathLibrary::Now().GetMinute();
	TimeText->SetText(FText::Format(FText::FromString("{0}:{1}"), FText::AsNumber(hour), FText::AsNumber(min)));

	HiText->SetText(InText);

	FString temp = "[";
	FString EnumName = "EChatTypes";
	//通过ChatType获取对应的Name
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	temp.Append(EnumPtr->GetEnumName(static_cast<int32>(ChatType)));
	temp.Append("]");
	Prefix->SetText(FText::FromString(temp));
}
