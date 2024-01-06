// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/XBlasterPlayerController.h"
#include "HUD/XBlasterHUD.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/XCharacter.h"

void AXBlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	XBlasterHUD = Cast<AXBlasterHUD>(GetHUD());
}

void AXBlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	//���ؿͻ��˵Ŀ�����
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AXBlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	//����Ϸ������ÿ��һ��ʱ��ͬ��������ʱ�䵽�ͻ���
	CheckTimeSync(DeltaTime);
}

void AXBlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	//����Ϸ������ÿ��һ��ʱ��ͬ��������ʱ�䵽�ͻ���
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AXBlasterPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetSeverTime());
	//��SecondsLeft����һ�β����ʱ����UI;
	if (SecondsLeft != CountDownInt)
	{
		SetHUDGameTime(MatchTime - GetSeverTime());
	}
	CountDownInt = SecondsLeft;
}

//sync SeverTime ClientTime
//Server Run GetWorld()->GetTimeSeconds() is SeverCurrentTime
void AXBlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequese)
{
	//��ȡ����ʱ��������ʱ��
	float SeverTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequese, SeverTimeOfReceipt);
}
//Client Run GetWorld()->GetTimeSeconds() is ClientCurrentTime
void AXBlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	//����RTT
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	//ServerCurrentTime
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AXBlasterPlayerController::GetSeverTime()
{
	//����ÿ������ڷ�������
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();

	}	
	else 
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	} 
}

void AXBlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->HealthBar &&
		XBlasterHUD->CharacterOverlayWdg->HealthText;
	if (bHUDvalid)
	{
		const float HealthPercent = Health / MaxHealth;
		XBlasterHUD->CharacterOverlayWdg->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		XBlasterHUD->CharacterOverlayWdg->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AXBlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AXCharacter* XCharacter = Cast<AXCharacter>(InPawn);

	if (XCharacter && XCharacter->GetPropertyComp())
	{
		SetHUDHealth(XCharacter->GetPropertyComp()->GetHealth(), XCharacter->GetPropertyComp()->GetMaxHealth());
	}
}

void AXBlasterPlayerController::SetHUDScore(float Score)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->ScoreAmount;
	if (bHUDvalid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		XBlasterHUD->CharacterOverlayWdg->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void AXBlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->DefeatsAmount;
	if (bHUDvalid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Defeats));
		XBlasterHUD->CharacterOverlayWdg->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void AXBlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->WeaponAmmoAmount;
	if (bHUDvalid)
	{
		FString WeaponAmmoText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Ammo));
		XBlasterHUD->CharacterOverlayWdg->WeaponAmmoAmount->SetText(FText::FromString(WeaponAmmoText));
	}
}

void AXBlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->CarriedAmmoAmount;
	if (bHUDvalid)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Ammo));
		XBlasterHUD->CharacterOverlayWdg->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void AXBlasterPlayerController::SetHUDGameTime(float CountDownTime)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->GameTimeText;
	if (bHUDvalid)
	{
		//��ȡʱ��
		int32 minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 seconds = CountDownTime - minutes * 60;

		FString GameTimeText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds);
		XBlasterHUD->CharacterOverlayWdg->GameTimeText->SetText(FText::FromString(GameTimeText));
	}
}
