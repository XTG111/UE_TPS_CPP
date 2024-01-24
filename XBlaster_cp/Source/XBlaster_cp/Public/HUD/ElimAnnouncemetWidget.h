// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncemetWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UElimAnnouncemetWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	//…Ë÷√
	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	UPROPERTY(meta = (BindWidget))
		class UHorizontalBox* ElimAnnoceBox;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ElimAnnoceText;
	
};
