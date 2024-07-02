// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/XBlasterHUD.h"
#include "HUD/CharacterOverlayWidget.h"
#include "GameFramework/PlayerController.h"
#include "HUD/AnnouncementWidget.h"
#include "HUD/ElimAnnouncemetWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ChatSystem/Widget/XChatWidget.h"

void AXBlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AXBlasterHUD::AddCharacterOverlay()
{
	PlayerController = PlayerController == nullptr ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && CharacterOverlayWdgClass)
	{
		CharacterOverlayWdg = CreateWidget<UCharacterOverlayWidget>(PlayerController, CharacterOverlayWdgClass);
		CharacterOverlayWdg->AddToViewport();
	}
}

void AXBlasterHUD::AddChatUI()
{
	PlayerController = PlayerController == nullptr ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && ChatBaseUIClass)
	{
		ChatWdg = CreateWidget<UXChatWidget>(PlayerController, ChatBaseUIClass);
		ChatWdg->AddToViewport();
	}
}

void AXBlasterHUD::AddAnnouncement()
{
	PlayerController = PlayerController == nullptr ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && AnnouncementClass)
	{
		AnnouncementWdg = CreateWidget<UAnnouncementWidget>(PlayerController, AnnouncementClass);
		AnnouncementWdg->AddToViewport();
	}
}

void AXBlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	PlayerController = PlayerController == nullptr ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncemetWidget* ElimAnnouncementWdg = CreateWidget<UElimAnnouncemetWidget>(PlayerController, ElimAnnouncementClass);
		if (ElimAnnouncementWdg)
		{
			ElimAnnouncementWdg->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWdg->AddToViewport();

			//向上滚动
			for (UElimAnnouncemetWidget* msg : ElimMsg)
			{
				//利用水平框的参数
				if (msg && msg->ElimAnnoceBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(msg->ElimAnnoceBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}

				}

			}

			ElimMsg.Add(ElimAnnouncementWdg);
			//调用定时器
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinish"), ElimAnnouncementWdg);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false
			);
		}
	}
}

void AXBlasterHUD::ElimAnnouncementTimerFinish(UElimAnnouncemetWidget* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void AXBlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewPortSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
		const FVector2D ViewPortCenter(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewPortCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}


//计算实际准星绘制位置
void AXBlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor DrawCrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, DrawCrosshairColor);
}

void AXBlasterHUD::SetHUDPackage(const FHUDPackage& Package)
{
	HUDPackage = Package;
}
