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
#include "Camera/CameraComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	SetIsReplicated(true);
	BaseWalkSpeed = 350.f;
	AimWalkSpeed = 100.f;
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
	if (CharacterEx && CharacterEx->IsLocallyControlled())
	{
		//���׼��λ��
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		//����׼��
		SetHUDCrossHairs(DeltaTime);

		//�����ӽ�
		InterpFOV(DeltaTime);
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

		if (CharacterEx->GetFollowCamera())
		{
			DefaultFOV = CharacterEx->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}	
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//Ϊ�˱��������ӳ٣����¶������ӳ�
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
		//ʰȡ�������л�����
		CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharacterEx->bUseControllerRotationYaw = true;
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || CharacterEx == nullptr) return;
	CharacterEx->GetWorldTimerManager().SetTimer(FireTime, this, &UCombatComponent::FireTimeFinished, EquippedWeapon->FireDelay);
}
//����ʱ�������ٵ���ControlFire�൱��һ��ѭ��
void UCombatComponent::FireTimeFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFired && EquippedWeapon->bAutomatic)
	{
		ControlFire(bFired);
	}
}

void UCombatComponent::ControlFire(bool bPressed)
{
	if (bCanFire)
	{
		
		//UE_LOG(LogTemp, Warning, TEXT("Check:%d"), CharacterEx->Check);
		bCanFire = false;
		if (bFired)
		{
			//CharacterEx->Check = 20.0f;
			ServerFire(bFired, HitTarget);
			if (EquippedWeapon)
			{
				CrosshairShootingFactor = 0.75f;
			}
		}
		StartFireTimer();
	}

}

void UCombatComponent::IsFired(bool bPressed)
{
	bFired = bPressed;
	ControlFire(bFired);
}

//RPC,�����ͨ��repNotify����ֵ�ı���������������ϲ��ܹ۲���
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
	//�ӿڴ�С
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//׼��λ����Ļ�������� ֻ���ӿ�����
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	//��Ҫת������������
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//���߼�������յ�
		FVector Start = CrosshairWorldPosition;
		if (CharacterEx)
		{
			float DistanceToCharacter = (CharacterEx->GetFollowCamera()->GetComponentLocation() - CharacterEx->GetActorLocation()).Size();
			//Start = CrosshairWorldPosition * (DistanceToCharacter + 100.f);
		}
		FVector End = Start + CrosshairWorldDirection * 5000.0f;

		//FVector Crosshair_Location = CharacterEx->GetFollowCamera()->GetComponentLocation() + CharacterEx->GetControlRotation().Vector() * 5000;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(CharacterEx);

		bool IsHit = GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility,Params);
		if (!IsHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}

		//�����ж���̳���InteractWithCrosshairInterface�ӿ�ʱ���ı�׼����ɫ
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

//���׼��
void UCombatComponent::SetHUDCrossHairs(float Deltatime)
{
	if (CharacterEx == nullptr || CharacterEx->Controller == nullptr) return;

	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;

	if (XBlasterPlayerController)
	{
		XBlasterHUD = XBlasterHUD == nullptr ? Cast<AXBlasterHUD>(XBlasterPlayerController->GetHUD()) : XBlasterHUD;
		if (XBlasterHUD)
		{
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
			//����Spread calculate crosshair spread
			//�����ٶ�ӳ��0-1 [0,GetMaxWalkSpeed]
			FVector2D WalkSpeedRange(0.f, CharacterEx->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityRange(0.f, 1.f);
			FVector Velocity = CharacterEx->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityRange, Velocity.Size());

			//���ڿ���ʱ��׼�ǵ���ɢ
			if (CharacterEx->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, Deltatime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, Deltatime, 30.f);
			}

			//��׼ʱ׼�Ǳ任
			if (bUnderAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, Deltatime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, Deltatime, 30.f);
			}

			//�����Ӧ�ò�ֵ��0
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, Deltatime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			XBlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

//��ֵ�����ӽ�
void UCombatComponent::InterpFOV(float Deltatime)
{
	if (EquippedWeapon == nullptr) return;

	//����������ZoomFOV��ZoomInterSpeed
	if (bUnderAiming)
	{
		//������������׼���ӽǱ任�����������õ���Ұ�ͱ仯�ٶ�
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->ZoomFOV, Deltatime, EquippedWeapon->ZoomInterpSpeed);
	}
	else
	{
		//������ȡ����׼���ӽǽ���ΪĬ�ϲ���������Ĭ�����õı仯�ٶ�
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, Deltatime, ZoomInterpSpeed);
	}
	if (CharacterEx && CharacterEx->GetFollowCamera())
	{
		CharacterEx->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::EquipWeapon(AWeaponParent* WeaponToEquip)
{
	if (CharacterEx == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	//ʹ�ù��������������λ��
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	//����
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, CharacterEx->GetMesh());
	}
	//SetOwner�е��β�Owner���������UE�Ѿ�Ĭ���˽������Ը���
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	EquippedWeapon->SetOwner(CharacterEx);

	//ʰȡ�������л�����
	CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterEx->bUseControllerRotationYaw = true;
}