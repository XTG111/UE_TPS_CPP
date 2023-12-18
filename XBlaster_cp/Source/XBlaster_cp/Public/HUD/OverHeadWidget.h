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

	//�ı���ģ��
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* DisplayText;

protected:
	//����ӵ�ǰ�ؿ��뿪ʱ�Ĳ���
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	//��ʾ�ı�
	void SetDisplayeText(FString TextToDisplay);

	//�жϽ�ɫ
	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);
};
