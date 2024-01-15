// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/XPropertyComponent.h"
#include "Net/UnrealNetWork.h"
#include "Character/XCharacter.h"
#include "GameMode/XBlasterGameMode.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UXPropertyComponent::UXPropertyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

void UXPropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UXPropertyComponent, Health);
	DOREPLIFETIME(UXPropertyComponent, Shield);
}


// Called when the game starts
void UXPropertyComponent::BeginPlay()
{
	Super::BeginPlay();
	XCharacter = Cast<AXCharacter>(GetOwner());

	//
}

// Called every frame
void UXPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UXPropertyComponent::ReceivedDamage(float Damage, AController* InstigatorController)
{
	//当角色死亡后，就不能再次接受伤害
	if (XCharacter->GetbElimed()) return;

	//没有护盾的扣血量
	float DamageToHealth = Damage;

	//添加护盾对血量的修改
	if (Shield >= 0)
	{
		//当护盾量大于伤害
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MAXShield);
			DamageToHealth = 0.f;
		}
		else
		{
			Shield = 0.f;
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MAXHealth);
	XCharacter->UpdateHUDHealth();
	XCharacter->UpdateHUDShield();
	//将受击动画的播放改到这里，降低RPC调用的负担
	XCharacter->PlayHitReactMontage();

	if (Health == 0.0f)
	{
		AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();
		if (XBlasterGameMode && XCharacter)
		{	
			AXBlasterPlayerController* AttackerContorller = Cast<AXBlasterPlayerController>(InstigatorController);
			XBlasterGameMode->PlayerEliminated(XCharacter, XCharacter->GetXBlasterPlayerCtr(), AttackerContorller);
		}
	}
}

void UXPropertyComponent::OnRep_HealthChange(float LastHealth)
{
	XCharacter->UpdateHUDHealth();
	//避免治疗时播放受击动画
	if (Health < LastHealth)
	{
		XCharacter->PlayHitReactMontage();
	}
}

void UXPropertyComponent::OnRep_SheildChange(float LastShield)
{
	XCharacter->UpdateHUDShield();
	if (Shield < LastShield)
	{
		XCharacter->PlayHitReactMontage();
	}
}

void UXPropertyComponent::HealCharacter(float HealAmount, float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	Healingrate = HealAmount / HealingTime;
}

void UXPropertyComponent::ShieldReplenish(float ShieldAmount, float ShealingTime)
{
	bShield = true;
	Shieldgrate = ShieldAmount / ShealingTime;
	AmountToShield += ShieldAmount;
}

void UXPropertyComponent::SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (XCharacter == nullptr) return;
	XCharacter->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UXPropertyComponent::ResetSpeed, BuffTime);
	if (XCharacter->GetCharacterMovement())
	{
		XCharacter->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		XCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticasatSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UXPropertyComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UXPropertyComponent::ResetSpeed()
{
	if (XCharacter == nullptr) return;
	if (XCharacter->GetCharacterMovement())
	{
		XCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		XCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	}
	MulticasatSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
	XCharacter->GetWorldTimerManager().ClearTimer(SpeedBuffTimer);
}

void UXPropertyComponent::MulticasatSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (XCharacter && XCharacter->GetCharacterMovement())
	{
		XCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		XCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

//弹跳
void UXPropertyComponent::JumpBuff(float JumpZHight, float BuffTime)
{
	if (XCharacter == nullptr) return;
	XCharacter->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UXPropertyComponent::ResetJump, BuffTime);
	if (XCharacter->GetCharacterMovement())
	{
		XCharacter->GetCharacterMovement()->JumpZVelocity = JumpZHight;
	}
	MulticasatJumpBuff(JumpZHight);
}

void UXPropertyComponent::SetInitialJump(float JumpZHight)
{
	InitialJumpZHight = JumpZHight;
}

void UXPropertyComponent::ResetJump()
{
	if (XCharacter == nullptr) return;
	if (XCharacter->GetCharacterMovement())
	{
		XCharacter->GetCharacterMovement()->JumpZVelocity = InitialBaseSpeed;
	}
	MulticasatJumpBuff(InitialJumpZHight);
	XCharacter->GetWorldTimerManager().ClearTimer(JumpBuffTimer);
}

void UXPropertyComponent::MulticasatJumpBuff_Implementation(float JumpZHight)
{
	if (XCharacter && XCharacter->GetCharacterMovement()) 
	{
		XCharacter->GetCharacterMovement()->JumpZVelocity = JumpZHight;
	}
}


void UXPropertyComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || XCharacter == nullptr || XCharacter->IsElimmed()) return;

	const float HealthisFrame = Healingrate * DeltaTime;
	//增加血量
	Health = FMath::Clamp(Health + HealthisFrame, 0.f, MAXHealth);
	if (XCharacter->HasAuthority())
	{
		XCharacter->UpdateHUDHealth();
	}
	AmountToHeal -= HealthisFrame;
	if (AmountToHeal <= 0.f || Health >= MAXHealth)
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UXPropertyComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShield || XCharacter == nullptr || XCharacter->IsElimmed()) return;

	const float ShieldthisFrame = Shieldgrate * DeltaTime;
	//增加血量
	Shield = FMath::Clamp(Shield + ShieldthisFrame, 0.f, MAXShield);
	if (XCharacter->HasAuthority())
	{
		XCharacter->UpdateHUDShield();
	}
	AmountToShield -= ShieldthisFrame;
	if (AmountToShield <= 0.f || Shield >= MAXShield)
	{
		bShield = false;
		AmountToShield = 0.f;
	}
}

