// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/XBlasterPlayerController.h"
#include "HUD/XBlasterHUD.h"
#include "HUD/CharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/XCharacter.h"
#include "Net/UnrealNetWork.h"
#include "GameMode/XBlasterGameMode.h"
#include "HUD/AnnouncementWidget.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterComponent/CombatComponent.h"
#include "Weapon/WeaponParent.h"
#include "GameState/XBlasterGameState.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "Components/Image.h"
#include "HUD/ReturnToMainMenuWidget.h"
#include "XBlaster_cp/XTypeHeadFile/Announcement.h"
#include "BlasterComponent/XChatComponent.h"

AXBlasterPlayerController::AXBlasterPlayerController()
{
	ChatComponent = CreateDefaultSubobject<UXChatComponent>(TEXT("ChatComponent"));
	ChatComponent->SetIsReplicated(true);
}

void AXBlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AXBlasterPlayerController, MatchState);
	DOREPLIFETIME(AXBlasterPlayerController, bShowTeamScores);
}

void AXBlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &AXBlasterPlayerController::ShowReturnToMainMenu);
	InputComponent->BindAction("Chat", IE_Pressed, this, &AXBlasterPlayerController::BeginChat);
}

void AXBlasterPlayerController::ShowReturnToMainMenu()
{
	//TODO 展示退出界面
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenuWidget>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSet();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AXBlasterPlayerController::OnRep_ShowTeamsScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

void AXBlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	XBlasterHUD = Cast<AXBlasterHUD>(GetHUD());
	ServerCheckMatchState();
}


void AXBlasterPlayerController::PollInit()
{
	if (CharacterOverlayWdg == nullptr)
	{
		if (XBlasterHUD && XBlasterHUD->CharacterOverlayWdg)
		{
			CharacterOverlayWdg = XBlasterHUD->CharacterOverlayWdg;
			if (CharacterOverlayWdg)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				//当UI初始化成功，直接调用角色战斗组件中的手雷数量
				AXCharacter* XCharacter = Cast<AXCharacter>(GetPawn());
				if (XCharacter && XCharacter->GetCombatComp() && bInitializeGrenade)
				{
					SetHUDGrenadeAmount(XCharacter->GetCombatComp()->GetGrenades());
				}
			}
		}
	}
}

//Server Time And State Is Server
void AXBlasterPlayerController::ServerCheckMatchState_Implementation()
{
	AXBlasterGameMode* GameMode = Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CoolDownTime = GameMode->CoolDownTime;
		//OnMatchStateSet(MatchState);
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CoolDownTime);
	}
}

//Client Accept Server's Time And State to fix Client Time And State
void AXBlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CoolDownTime = CoolDown;
	OnMatchStateSet(MatchState);

	if (XBlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		XBlasterHUD->AddAnnouncement();
	}
}


void AXBlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	//本地客户端的控制器
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AXBlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	//在游戏过程中每隔一段时间同步服务器时间到客户端
	CheckTimeSync(DeltaTime);
	PollInit();

	//更新Ping的冷却时间计数
	PlayHighPingAnim(DeltaTime);
}

void AXBlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	//在游戏过程中每隔一段时间同步服务器时间到客户端
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AXBlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetSeverTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetSeverTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::CoolDown)
	{
		TimeLeft = CoolDownTime + WarmupTime + MatchTime - GetSeverTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	//如果是服务器上运行那么直接调用GameMode里面的存储时间的变量，来设置SecondsLeft
	if (HasAuthority())
	{
		XBlasterGameMode = XBlasterGameMode == nullptr ? Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this)) : XBlasterGameMode;
		if (XBlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(XBlasterGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}

	//当SecondsLeft和上一次不相等时更新UI;
	if (SecondsLeft != CountDownInt)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown)
		{
			SetHUDAnnouncementCountDown(SecondsLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDGameTime(SecondsLeft);
		}
	}
	CountDownInt = SecondsLeft;
}

//sync SeverTime ClientTime
//Server Run GetWorld()->GetTimeSeconds() is SeverCurrentTime
void AXBlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequese)
{
	//获取接受时服务器的时刻
	float SeverTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequese, SeverTimeOfReceipt);
}
//Client Run GetWorld()->GetTimeSeconds() is ClientCurrentTime
void AXBlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	//计算RTT
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = RoundTripTime * 0.5f;
	//ServerCurrentTime
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}


float AXBlasterPlayerController::GetSeverTime()
{
	//如果该控制器在服务器上
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
	//如果没有绘制成功，那么将值存入到变量中，之后在PollInit中再次绘制
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AXBlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->ShieldBar &&
		XBlasterHUD->CharacterOverlayWdg->ShieldText;
	if (bHUDvalid)
	{
		const float ShieldPercent = Shield / MaxShield;
		XBlasterHUD->CharacterOverlayWdg->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		XBlasterHUD->CharacterOverlayWdg->ShieldText->SetText(FText::FromString(ShieldText));
	}
	//如果没有绘制成功，那么将值存入到变量中，之后在PollInit中再次绘制
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
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
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
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
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
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
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
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
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
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
		if (CountDownTime < 0.f)
		{
			XBlasterHUD->CharacterOverlayWdg->GameTimeText->SetText(FText());
			return;
		}


		//获取时间
		int32 minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 seconds = CountDownTime - minutes * 60;

		FString GameTimeText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds);
		XBlasterHUD->CharacterOverlayWdg->GameTimeText->SetText(FText::FromString(GameTimeText));
	}
}

void AXBlasterPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->AnnouncementWdg &&
		XBlasterHUD->AnnouncementWdg->WarmUpTimeText;
	if (bHUDvalid)
	{
		//如果时间为负数则隐藏
		if (CountDownTime < 0.f)
		{
			XBlasterHUD->AnnouncementWdg->WarmUpTimeText->SetText(FText());
			return;
		}

		//获取时间
		int32 minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 seconds = CountDownTime - minutes * 60;

		FString WarmUpTimeText = FString::Printf(TEXT("%02d:%02d"), minutes, seconds);
		XBlasterHUD->AnnouncementWdg->WarmUpTimeText->SetText(FText::FromString(WarmUpTimeText));
	}
}

void AXBlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	//当状态是InProgress时，调用UI绘制CharacterOverlay
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
		//检测是否是团队游戏
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void AXBlasterPlayerController::OnRep_MatchState()
{
	//当状态是InProgress时，调用UI绘制
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void AXBlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
	if (XBlasterHUD)
	{
		if (XBlasterHUD->CharacterOverlayWdg == nullptr)
		{
			XBlasterHUD->AddCharacterOverlay();
			XBlasterHUD->AddChatUI();
		}
		if (XBlasterHUD->AnnouncementWdg)
		{
			XBlasterHUD->AnnouncementWdg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (HasAuthority())
		{
			if (bTeamsMatch)
			{
				InitTeamScores();
			}
			else
			{
				HideTeamScores();
			}
		}
	}
}

void AXBlasterPlayerController::HandleCoolDown()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
	if (XBlasterHUD)
	{
		XBlasterHUD->CharacterOverlayWdg->RemoveFromParent();
		if (XBlasterHUD->AnnouncementWdg)
		{
			XBlasterHUD->AnnouncementWdg->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			if (XBlasterHUD->AnnouncementWdg->AnnouncementText)
			{
				XBlasterHUD->AnnouncementWdg->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}

			AXBlasterGameState* XBlasterGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
			AXBlasterPlayerState* XBlasterPlayerState = GetPlayerState< AXBlasterPlayerState >();

			if (XBlasterGameState && XBlasterPlayerState && XBlasterHUD->AnnouncementWdg->InfoText)
			{
				TArray<AXBlasterPlayerState*> TopPlayers = XBlasterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamInfoText(XBlasterGameState) : GetInfoText(TopPlayers);
				XBlasterHUD->AnnouncementWdg->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AXCharacter* XCharacter = Cast<AXCharacter>(GetPawn());
	if (XCharacter)
	{
		XCharacter->bDisableGamePlay = true;
		if (XCharacter->GetCombatComp())
		{
			XCharacter->GetCombatComp()->IsFired(false);
		}
	}
}

FString AXBlasterPlayerController::GetInfoText(const TArray<class AXBlasterPlayerState*>& Players)
{
	AXBlasterPlayerState* XBlasterPlayerState = GetPlayerState< AXBlasterPlayerState >();
	if (XBlasterPlayerState == nullptr) return FString();

	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == XBlasterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::Playerstiedforthewin;
		InfoTextString.Append(FString("\n"));
		for (auto WinnerPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *WinnerPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString AXBlasterPlayerController::GetTeamInfoText(const AXBlasterGameState* XBlasterGameState)
{
	if (XBlasterGameState == nullptr) return FString();
	FString InfoTextString;
	const int32 RedTeamScore = XBlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = XBlasterGameState->BlueTeamScore;

	if (RedTeamScore == BlueTeamScore && RedTeamScore == 0.f)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::Teamstiedforthewin;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}

//Is Ping Too High _Implementation
void AXBlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}


void AXBlasterPlayerController::HighPingWarning()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->WifiWARNING &&
		XBlasterHUD->CharacterOverlayWdg->HighPingAnim;
	if (bHUDvalid)
	{
		XBlasterHUD->CharacterOverlayWdg->WifiWARNING->SetOpacity(1.f);
		XBlasterHUD->CharacterOverlayWdg->PlayAnimation(
			XBlasterHUD->CharacterOverlayWdg->HighPingAnim,
			0.f,
			5
		);
	}
}

void AXBlasterPlayerController::StopHighPingWarning()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->WifiWARNING &&
		XBlasterHUD->CharacterOverlayWdg->HighPingAnim;
	if (bHUDvalid)
	{
		XBlasterHUD->CharacterOverlayWdg->WifiWARNING->SetOpacity(1.f);
		if (XBlasterHUD->CharacterOverlayWdg->IsAnimationPlaying(XBlasterHUD->CharacterOverlayWdg->HighPingAnim))
		{
			XBlasterHUD->CharacterOverlayWdg->StopAnimation(XBlasterHUD->CharacterOverlayWdg->HighPingAnim);
		}
	}
}

void AXBlasterPlayerController::PlayHighPingAnim(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		//如果冷却时间过了，那么又可以继续显示pingwarning

		//首先需要获得ping的大小 -- 可以利用我们计算的RoundTripTime/2来获得 或者 PlayerState->GetPing()
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			//实际Ping 的 1/4
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				PingAnimationRunningTime = 0.f;
				HighPingWarning();
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimPlay = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->HighPingAnim &&
		XBlasterHUD->CharacterOverlayWdg->IsAnimationPlaying(XBlasterHUD->CharacterOverlayWdg->HighPingAnim);
	if (bHighPingAnimPlay)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AXBlasterPlayerController::SetHUDGrenadeAmount(int32 GrenadeAmount)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->GrenadeAmount;
	if (bHUDvalid)
	{
		FString GrenadeText = FString::Printf(TEXT("%d"), FMath::FloorToInt(GrenadeAmount));
		XBlasterHUD->CharacterOverlayWdg->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitializeGrenade = true;
		HUDGrenadeAmount = GrenadeAmount;
	}
}

void AXBlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AXBlasterPlayerController::HideTeamScores()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore &&
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore &&
		XBlasterHUD->CharacterOverlayWdg->ScoreSpacetext;
	if (bHUDvalid)
	{
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore->SetText(FText());
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore->SetText(FText());
		XBlasterHUD->CharacterOverlayWdg->ScoreSpacetext->SetText(FText());
	}
}

void AXBlasterPlayerController::InitTeamScores()
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore &&
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore &&
		XBlasterHUD->CharacterOverlayWdg->ScoreSpacetext;
	if (bHUDvalid)
	{
		FString Zero("0");
		FString Spacer("|");
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore->SetText(FText::FromString(Zero));
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore->SetText(FText::FromString(Zero));
		XBlasterHUD->CharacterOverlayWdg->ScoreSpacetext->SetText(FText::FromString(Spacer));
	}
}

void AXBlasterPlayerController::SetHUDRedTeamScores(int32 RedTeamScore)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore;
	if (bHUDvalid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedTeamScore);
		XBlasterHUD->CharacterOverlayWdg->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AXBlasterPlayerController::SetHUDBlueTeamScores(int32 BlueTeamScore)
{
	XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;

	bool bHUDvalid = XBlasterHUD &&
		XBlasterHUD->CharacterOverlayWdg &&
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore;
	if (bHUDvalid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueTeamScore);
		XBlasterHUD->CharacterOverlayWdg->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

//ClientRPC
void AXBlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* SelfState = GetPlayerState<APlayerState>();

	if (Attacker && Victim && SelfState)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			if (Attacker == SelfState && Victim != SelfState)
			{
				XBlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			else if (Attacker != SelfState && Victim == SelfState)
			{
				XBlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			else if (Attacker == SelfState && Victim == SelfState)
			{
				XBlasterHUD->AddElimAnnouncement("You", "You");
				return;
			}
			else if (Attacker == Victim && Victim != SelfState)
			{
				XBlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Self");
				return;
			}
			else
			{
				XBlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
			}
		}
	}
}


void AXBlasterPlayerController::BeginChat()
{
	ChatComponent->ToggleChatWindow();
}