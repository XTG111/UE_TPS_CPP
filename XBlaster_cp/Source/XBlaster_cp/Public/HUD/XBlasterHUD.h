// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "XBlasterHUD.generated.h"


//存储准星的结构体
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:

	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;

	//准星的扩展
	float CrosshairSpread;
	//准星的颜色
	FLinearColor CrosshairColor;
};
/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AXBlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	//人物属性
	UPROPERTY(EditAnywhere,Category = "Player State")
		TSubclassOf<class UUserWidget> CharacterOverlayWdgClass;
	UPROPERTY()
		class UCharacterOverlayWidget* CharacterOverlayWdg;

	UPROPERTY(EditAnywhere, Category = "Warm State")
		TSubclassOf<class UUserWidget> AnnouncementClass;
	UPROPERTY()
		class UAnnouncementWidget* AnnouncementWdg;

	UPROPERTY(EditAnywhere)
		TSubclassOf<class UElimAnnouncemetWidget> ElimAnnouncementClass;

protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	UPROPERTY()
	 class APlayerController* PlayerController;

	//绘制准星，Spread用于控制准星的变化
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor DrawCrosshairColor);

	UPROPERTY(EditAnywhere)
		float CrosshairSpreadMax = 16.f;

	//控制一条击杀通知的显示时间
	UPROPERTY(EditAnywhere)
		float ElimAnnouncementTime = 3.f;
	//定时器回调函数
	UFUNCTION()
		void ElimAnnouncementTimerFinish(UElimAnnouncemetWidget* MsgToRemove);

	//存储所有的击杀信息
	UPROPERTY()
		TArray<UElimAnnouncemetWidget*> ElimMsg;

public:
	void SetHUDPackage(const FHUDPackage& Package);
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker,FString Victim);
	
};
