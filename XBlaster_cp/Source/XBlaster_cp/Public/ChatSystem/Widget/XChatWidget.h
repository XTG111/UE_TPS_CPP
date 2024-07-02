// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../XTypeHeadFile/XChatEnum.h"
#include "XChatWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleChat, bool, btoggle);

UCLASS()
class XBLASTER_CP_API UXChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;

	void ToggleChatBase(bool btoggle);
	UPROPERTY(meta = (BindWidget))
		class UXTextFieldWidget* TextField;
	UPROPERTY(meta = (BindWidget))
		class UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
		class UWidgetAnimation* Toggle;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
		class UWidgetAnimation* DelayToggle;

	UFUNCTION()
		void OnSendText(const FText& text);

	bool RefindCommands(const FText& InText, ECommandTypes CTP, FString& Arguments);

	//将输入的文本转换为小写
	void RebuildCommands(const FText& InText, FString& Output, FString& res);


	enum EChatTypes ChatType;
	int ReaserchLen;

	class UXChatComponent* GetNowUserComponent(AActor* Owner);

	FOnToggleChat OnToggleChat;
};
