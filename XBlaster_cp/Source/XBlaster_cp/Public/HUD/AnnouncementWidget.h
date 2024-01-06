// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* WarmUpTimeText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* AnnouncementText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* InfoText;
	
};
