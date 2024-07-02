// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../XTypeHeadFile/XChatEnum.h"
#include "XTextWidget.generated.h"

/**
 *
 */
UCLASS()
class XBLASTER_CP_API UXTextWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
		class UButton* ShowButton;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* TimeText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Prefix;
	UPROPERTY(meta = (BindWidget))
		UButton* PingButton;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* PlayerNameText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* HiText;

public:
	FText PlayerName;
	FText InText;
	EMessageTypes MessType;
	EChatTypes ChatType;
};
