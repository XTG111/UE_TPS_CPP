// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ElimAnnouncemetWidget.h"
#include "Components/TextBlock.h"

void UElimAnnouncemetWidget::SetElimAnnouncementText(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s killed %s"), *AttackerName, *VictimName);
	if (ElimAnnoceText)
	{
		ElimAnnoceText->SetText(FText::FromString(ElimAnnouncementText));
	}
}

