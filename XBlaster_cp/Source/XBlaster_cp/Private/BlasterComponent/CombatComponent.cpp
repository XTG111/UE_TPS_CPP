// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/CombatComponent.h"
#include "Weapon/WeaponParent.h"
#include "Character/XCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetWork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "HUD/XBlasterHUD.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	BaseWalkSpeed = 350.f;
	AimWalkSpeed = 100.f;
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...

	SetHUDCrossHairs(DeltaTime);

	if (CharacterEx && CharacterEx->IsLocallyControlled())
	{
		FHitResult HitResult;

		TraceUnderCrosshairs(HitResult);

		HitTarget = HitResult.ImpactPoint;
	}

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bUnderAiming);
	//DOREPLIFETIME(UCombatComponent, bFired);
	//DOREPLIFETIME(UCombatComponent, BaseWalkSpeed);
	//DOREPLIFETIME(UCombatComponent, AimWalkSpeed);
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//为了避免网络延迟，导致动画的延迟
	bUnderAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->MaxWalkSpeed = bUnderAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bUnderAiming = bIsAiming;
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->MaxWalkSpeed = bUnderAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && CharacterEx)
	{
		//拾取武器后切换控制
		CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharacterEx->bUseControllerRotationYaw = true;
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	}
}

void UCombatComponent::IsFired(bool bPressed)
{
	//bFired = bPressed;
	//开枪之前，先调用求位置的函数
	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);

	ServerFire(bPressed, HitResult.ImpactPoint);

}

//RPC,如果不通过repNotify进行值改变操作，其他机器上不能观测结果
void UCombatComponent::ServerFire_Implementation(bool bPressed, const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(bPressed, TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(bool bPressed, const FVector_NetQuantize& TraceHitTarget)
{
	bFired = bPressed;
	if (EquippedWeapon == nullptr) return;
	//UE_LOG(LogTemp, Warning, TEXT("AO_YAW:%d"), bFired);
	if (CharacterEx)
	{
		CharacterEx->PlayFireMontage(bUnderAiming);
		if (bFired)
		{
			EquippedWeapon->Fire(TraceHitTarget);
		}
		else
		{
			EquippedWeapon->WeaponMesh->Stop();
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	//视口大小
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//准星位置屏幕中心坐标 只是视口坐标
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	//需要转换到世界坐标
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		//射线检测起点和终点
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * 50000.0f;

		bool IsHit = GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (!IsHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}
	}
}

//添加准星
void UCombatComponent::SetHUDCrossHairs(float Deltatime)
{
	if (CharacterEx == nullptr || CharacterEx->Controller == nullptr) return;

	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;

	if (XBlasterPlayerController)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(XBlasterPlayerController->GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//设置Spread calculate crosshair spread
			//利用速度映射0-1 [0,GetMaxWalkSpeed]
			FVector2D WalkSpeedRange(0.f, CharacterEx->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityRange(0.f, 1.f);
			FVector Velocity = CharacterEx->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityRange, Velocity.Size());

			//当在空中时，准星的扩散
			if (CharacterEx->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, Deltatime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, Deltatime, 30.f);
			}

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;

			XBlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::EquipWeapon(AWeaponParent* WeaponToEquip)
{
	if (CharacterEx == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	//使用骨骼插槽设置武器位置
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	//附加
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, CharacterEx->GetMesh());
	}
	//SetOwner中的形参Owner这个函数，UE已经默认了进行属性复制
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	EquippedWeapon->SetOwner(CharacterEx);

	//拾取武器后切换控制
	CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterEx->bUseControllerRotationYaw = true;
}