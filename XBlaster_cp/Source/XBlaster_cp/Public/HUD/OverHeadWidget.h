// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	//文本框模块
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* DisplayText;

protected:
	//处理从当前关卡离开时的操作
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	//显示文本
	void SetDisplayeText(FString TextToDisplay);

	//判断角色
	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);
};
