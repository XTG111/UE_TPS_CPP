// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "XTextFieldWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSendText, const FText&, text);

UCLASS()
class XBLASTER_CP_API UXTextFieldWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
		class UWidgetAnimation* Toggle;
	UPROPERTY(meta = (BindWidget))
		class UEditableText* SendText;

	UFUNCTION(BlueprintCallable)
		void ToggleChat(bool btoggle);

	UFUNCTION(BlueprintCallable)
		void TextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	class UXChatComponent* GetNowUserComponent(AActor* Owner);

	FOnSendText OnSendText;

};
