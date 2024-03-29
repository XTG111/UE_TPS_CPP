// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlayWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UCharacterOverlayWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta =  (BindWidget))
		class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HealthText;
	UPROPERTY(meta = (BindWidget))
		class UProgressBar* ShieldBar;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* DefeatsAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* WeaponAmmoAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* CarriedAmmoAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* GameTimeText;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* GrenadeAmount;
	
	//绑定UI的动画和控件
	UPROPERTY(meta = (BindWidget))
		class UImage* WifiWARNING;
	//Transient:没有序列化到磁盘
	UPROPERTY(meta = (BindWidgetAnim), Transient)
		UWidgetAnimation* HighPingAnim;

	//队伍分数显示
	UPROPERTY(meta = (BindWidget))
		UTextBlock* BlueTeamScore;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* RedTeamScore;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreSpacetext;
};
