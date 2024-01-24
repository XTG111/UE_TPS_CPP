// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "XBlasterHUD.generated.h"


//�洢׼�ǵĽṹ��
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

	//׼�ǵ���չ
	float CrosshairSpread;
	//׼�ǵ���ɫ
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

	//��������
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

	//����׼�ǣ�Spread���ڿ���׼�ǵı仯
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor DrawCrosshairColor);

	UPROPERTY(EditAnywhere)
		float CrosshairSpreadMax = 16.f;

	//����һ����ɱ֪ͨ����ʾʱ��
	UPROPERTY(EditAnywhere)
		float ElimAnnouncementTime = 3.f;
	//��ʱ���ص�����
	UFUNCTION()
		void ElimAnnouncementTimerFinish(UElimAnnouncemetWidget* MsgToRemove);

	//�洢���еĻ�ɱ��Ϣ
	UPROPERTY()
		TArray<UElimAnnouncemetWidget*> ElimMsg;

public:
	void SetHUDPackage(const FHUDPackage& Package);
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker,FString Victim);
	
};
