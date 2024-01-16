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
#include "Sound/SoundCue.h"
#include "Character/XCharacterAnimInstance.h"
#include "Weapon/ProjectileActor.h"
#include "GameMode/XBlasterGameMode.h"

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
	DOREPLIFETIME(UCombatComponent, SecondWeapon);
	DOREPLIFETIME(UCombatComponent, bUnderAiming);
	//ֻ��Owner�ͻ��˸���
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, GrenadeAmount);
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
	//��ʼ����ɫ����ǹе�ı�����
	if (CharacterEx->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (CharacterEx == nullptr || EquippedWeapon == nullptr) return;
	//Ϊ�˱��������ӳ٣����¶������ӳ�
	bUnderAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->MaxWalkSpeed = bUnderAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	//ֻ�ڱ�������
	if (CharacterEx->IsLocallyControlled() && EquippedWeapon->WeaponType == EWeaponType::EWT_Snipper)
	{
		CharacterEx->ShowSnipperScope(bIsAiming);
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

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubMachineGun, SubMachineGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, ShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Snipper, SnipperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, GrenadeLauncherAmmo);
}

void UCombatComponent::DropEquippedWeapon()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	//��ǹ�����UI
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (CharacterEx->IsLocallyControlled() && XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(0);
		XBlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	EquippedWeapon = nullptr;
	CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterEx->bUseControllerRotationYaw = false;
	if(CharacterEx && !CharacterEx->HasAuthority()) ServerDropWeapon();
}

void UCombatComponent::ServerDropWeapon_Implementation()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	EquippedWeapon = nullptr;
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = true;
		CharacterEx->bUseControllerRotationYaw = false;
	}
}

void UCombatComponent::ChangeEquippedWeapon()
{
	//��ӵ����������
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
}

//���ӵ�����
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (CharacterEx == nullptr || CharacterEx->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	//����
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, CharacterEx->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (CharacterEx == nullptr || CharacterEx->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;

	bool bUsePistolSocket = EquippedWeapon->WeaponType == EWeaponType::EWT_Pistol || EquippedWeapon->WeaponType == EWeaponType::EWT_SubMachineGun;

	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");

	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(SocketName);
	//����
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, CharacterEx->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackPackage(AActor* ActorToAttach)
{
	if (CharacterEx == nullptr || CharacterEx->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	//����
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, CharacterEx->GetMesh());
	}
}

//��������UI
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	//����CarriedAmmo
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
	}
	//���Ʊ�����UI
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

//����װ����������Ч
void UCombatComponent::PlayEquipWeaponSound(AWeaponParent* WeaponToEquip)
{
	//����װ������ʱ����Ч
	if (CharacterEx && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, CharacterEx->GetActorLocation());
	}
}

void UCombatComponent::ReloadWeaponAutomatic()
{
	//auto reload when pickup an empty weapon if have carriedAmmo
	if (EquippedWeapon && EquippedWeapon->Ammo == 0)
	{
		ReloadWeapon();
	}
}

void UCombatComponent::EquipWeapon(AWeaponParent* WeaponToEquip)
{
	if (CharacterEx == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	//�����ʱ���ǿ���״̬��ô������װ������
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	//װ���ڶ�������
	if (EquippedWeapon != nullptr && SecondWeapon == nullptr)
	{
		EquipSecondWeapon(WeaponToEquip);
	}
	else
	{
		//װ����һ������
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	//ʰȡ�������л�����
	CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterEx->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponParent* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//�������е�����
	ChangeEquippedWeapon();
	//ʹ�ù��������������λ��
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//���ӵ�����
	AttachActorToRightHand(EquippedWeapon);
	//SetOwner�е��β�Owner���������UE�Ѿ�Ĭ���˽������Ը���
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	EquippedWeapon->SetOwner(CharacterEx);
	EquippedWeapon->SetHUDAmmo();
	//���±�������UI
	UpdateCarriedAmmo();
	//װ����Ч
	PlayEquipWeaponSound(WeaponToEquip);
	//�Զ�����
	ReloadWeaponAutomatic();
}

void UCombatComponent::EquipSecondWeapon(AWeaponParent* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//ʹ�ù��������������λ��
	SecondWeapon = WeaponToEquip;
	SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
	//���ӵ�����
	AttachActorToBackPackage(WeaponToEquip);
	//SetOwner�е��β�Owner���������UE�Ѿ�Ĭ���˽������Ը���
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	SecondWeapon->SetOwner(CharacterEx);
	//װ����Ч
	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::NewEquipWeapon()
{
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	AActor* MyOwner = GetOwner();
	//FVector Start; = EyeLocation
	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector End = EyeLocation + (EyeRotation.Vector() * 500);
	//ʹ����ɨ
	TArray<FHitResult> Hits;
	float Radius = 30.f;
	FCollisionShape Shape;
	Shape.SetSphere(30.0f);
	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, Shape);
	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;
	for (FHitResult Hit : Hits) {
		AActor* HitActor = Hit.GetActor();
		if (HitActor) {
			if (HitActor->Implements<UFObjectInterface>()) {
				APawn* MyPawn = Cast<APawn>(MyOwner);
				IFObjectInterface::Execute_FPickObject(HitActor, MyPawn);
				break;
			}
		}
		//DrawDebugSphere(GetWorld(), Hit.ImpactPoint, Radius, 32, LineColor, false, 2.0f);
	}
	//DrawDebugLine(GetWorld(), EyeLocation, End, LineColor, false, 2.0f, 0, 2.0f);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && CharacterEx && CharacterEx->IsLocallyControlled())
	{
		//����״̬�����ú͸��ӵ����ϵĲ��������ж��Ⱥ�ģ������ڿͻ�����Ҳ�����޸ı�֤��ȷ��
		//ʰȡ�������л�����
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		//����װ������ʱ����Ч
		PlayEquipWeaponSound(EquippedWeapon);
		CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharacterEx->bUseControllerRotationYaw = true;
		EquippedWeapon->SetHUDAmmo();
	}
	if (CharacterEx && CharacterEx->IsLocallyControlled())
	{
		UpdateCarriedAmmo();
		XBlasterPlayerController->SetHUDWeaponAmmo(EquippedWeapon->Ammo);
	}

	if (EquippedWeapon == nullptr)
	{
		CombatState = ECombatState::ECS_Unoccupied;
		if (CharacterEx && !CharacterEx->IsLocallyControlled())
		{
			CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = true;
			CharacterEx->bUseControllerRotationYaw = false;
		}
	}
}

void UCombatComponent::OnRep_SecondWeapon()
{
	if (SecondWeapon && CharacterEx)
	{
		//����״̬�����ú͸��ӵ����ϵĲ��������ж��Ⱥ�ģ������ڿͻ�����Ҳ�����޸ı�֤��ȷ��
		//ʰȡ�������л�����
		SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
		AttachActorToBackPackage(SecondWeapon);
		//����װ������ʱ����Ч
		PlayEquipWeaponSound(SecondWeapon);
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
	////auro reload when no ammo in automatic Fire
	//if (EquippedWeapon->Ammo == 0)
	//{
	//	ReloadWeapon();
	//}
}

//ͬ��������
void UCombatComponent::OnRep_CarriedAmmo()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
		//XBlasterPlayerController->SetHUDWeaponAmmo(EquippedWeapon->Ammo);
	}
	//����ǹ����ת����ͬ�����ͻ���
	bool bJumpToShotGunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->WeaponType == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
	if (bJumpToShotGunEnd)
	{
		JumpToShotGunEnd();
	}
}

//Client use
void UCombatComponent::ReloadWeapon()
{
	if (EquippedWeapon)
	{
		bool AmmoRemain = EquippedWeapon->Ammo < EquippedWeapon->MaxAmmo;
		if (CarriedAmmo > 0 && AmmoRemain && CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerReload();
		}
	}
}

//Server use
void UCombatComponent::ServerReload_Implementation()
{
	if (CharacterEx == nullptr && EquippedWeapon == nullptr) return;
	CharacterEx->GetWorldTimerManager().ClearTimer(FireTime);
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

//Client Use ͨ��RPC�޸ĵ�CombatState���Ŷ���
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFired)
		{
			ControlFire(bFired);
		}
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_ThrowingGrenade: //OnRepNotify�������������ͻ���
		if (CharacterEx && !CharacterEx->IsLocallyControlled())
		{
			
			CharacterEx->PlayGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}

void UCombatComponent::FinishingReloading()
{
	if (CharacterEx == nullptr) return;
	if (CharacterEx->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		//�ڶ������Ž���֮�����
		UpdateAmmoAndCarriedAmmo();
	}
}

void UCombatComponent::UpdateAmmoAndCarriedAmmo()
{
	if (CharacterEx == nullptr && EquippedWeapon == nullptr) return;

	//�����ӵ�
	int32 ReloadAmount = AmmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		//���±������Լ�UI��ʾ�ı�����
		CarriedAmmoMap[EquippedWeapon->WeaponType] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
	}
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

//����ǹ�������ã�������֪ͨ����
void UCombatComponent::ShotGunUpdateAmmoAndCarriedAmmo()
{
	if (CharacterEx == nullptr && EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		//���±������Լ�UI��ʾ�ı�����
		CarriedAmmoMap[EquippedWeapon->WeaponType] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
	}

	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-1);

	//��������һ���ӵ��Ϳ��Կ�����
	bCanFire = true;
	//MaxAmmo 5
	//��������maxammo ���� û�б�����ô��ת
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		//Jump To End Section
		JumpToShotGunEnd();
	}
}

void UCombatComponent::JumpToShotGunEnd()
{
	UAnimInstance* AnimInstance = CharacterEx->GetMesh()->GetAnimInstance();
	if (AnimInstance && CharacterEx->GetRelodMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotGunReloadEnd"));
	}
}

void UCombatComponent::HandleReload()
{
	CharacterEx->PlayReloadMontage();
}

void UCombatComponent::ControlFire(bool bPressed)
{
	if (EquippedWeapon && HaveAmmoCanFire())
	{
		bCanFire = false;
		if (bPressed)
		{
			//������̫�涯���Լ��˺��ж�
			ServerFire(bPressed, HitTarget);
			if (EquippedWeapon)
			{
				CrosshairShootingFactor = 0.75f;
			}
		}
		//�����ӵ�
		StartFireTimer();
	}
	else if (EquippedWeapon && EquippedWeapon->Ammo == 0)
	{
		if (EquippedWeapon->DryFireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->DryFireSound, CharacterEx->GetActorLocation());
		}
	}
}

bool UCombatComponent::HaveAmmoCanFire()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	//��������ǹ�ĵ�����ǹ�ж�
	if (EquippedWeapon->Ammo > 0 && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->WeaponType == EWeaponType::EWT_ShotGun)
	{
		return true;
	}

	return EquippedWeapon->Ammo > 0 && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

//ִ�п�ǹ
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
	// 
	// ��������ǹ
	if (CharacterEx && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->WeaponType == EWeaponType::EWT_ShotGun)
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
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	//����������Ч�Ĳ���
	if (CharacterEx && CombatState == ECombatState::ECS_Unoccupied)
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
			//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
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

int32 UCombatComponent::AmmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	//������Ҫ������ٵ���
	int32 RoomInMag = EquippedWeapon->MaxAmmo - EquippedWeapon->Ammo;
	//��ȡ��ǰ�ı�����
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->WeaponType];
		//���������ͱ���������Сֵ��Ϊʵ�ʿ������ӵ��ӵ���
		int32 Least = FMath::Min(RoomInMag, AmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

//Server To Client
void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenade();
}

void UCombatComponent::UpdateHUDGrenade()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDGrenadeAmount(GrenadeAmount);
	}
}

//���ؿͻ��˵ĵ���
void UCombatComponent::ThrowGrenade()
{
	//���⵱ǰ���׵�����Ϊ0
	if (GrenadeAmount == 0) return;

	//�����ǰ״̬���ǿ��У��Ͳ��ܼ���ִ�У������ΰ���
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	//�޸�CombatState����ͨRepNotify�������ᴫ�ݸ��ͻ��ˣ�����ҪServerRPC���ͻ��˴��ݸ�������
	CombatState = ECombatState::ECS_ThrowingGrenade;

	//�����ǰ�ڱ��ؿͻ��ˣ���ô�����ӳٿ����Ȳ���mongtage����
	if (CharacterEx)
	{
		CharacterEx->PlayGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//����ServerRPCʹ�÷�������ִ�� !CharacterEx->HasAuthority()�����ظ�����
	if (CharacterEx && !CharacterEx->HasAuthority())
	{
		ServerThrowGrenade();
	}

	//����������ڷ������ϲ���ִ��ServerThrowGrenade();������Ҫ��������UI����
	if (CharacterEx && CharacterEx->HasAuthority())
	{
		//�ڷ������ϴ���������������
		GrenadeAmount = FMath::Clamp(GrenadeAmount - 1, 0, MaxGrenade);
		UpdateHUDGrenade();
	}
}

//�������ϵ�ʵ��
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	//�ڷ�������Ҳ��⣬��������
	if (GrenadeAmount == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (CharacterEx)
	{
		CharacterEx->PlayGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//�ڷ������ϴ���������������
	GrenadeAmount = FMath::Clamp(GrenadeAmount - 1, 0, MaxGrenade);
	UpdateHUDGrenade();
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (CharacterEx && CharacterEx->GetGrenadeComp())
	{
		CharacterEx->GetGrenadeComp()->SetVisibility(bShowGrenade);
	}
}

//��������
void UCombatComponent::SwapWeapon()
{
	AWeaponParent* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondWeapon;
	SecondWeapon = TempWeapon;

	//����
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	//Ammo��û��OnRep����������ͨ��EquippedWeapon������HUD�ĸ���
	EquippedWeapon->SetHUDAmmo();
	//���õ����ǿɸ��Ƶģ�������������Ը��¿ͻ��˵�HUD
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
	AttachActorToBackPackage(SecondWeapon);
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

void UCombatComponent::ShotGunShellReload()
{
	if (CharacterEx && CharacterEx->HasAuthority())
	{
		ShotGunUpdateAmmoAndCarriedAmmo();
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

//���������������
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	//ֻ�ڷ����������ɣ�Ȼ�󴫵ݸ������ͻ���
	if (CharacterEx && CharacterEx->IsLocallyControlled())
	{
		ServerLauncherGrenade(HitTarget);
	}
}

//Server use
void UCombatComponent::ServerLauncherGrenade_Implementation(const FVector_NetQuantize& Target)
{
	//ֻ�ڷ����������ɣ�Ȼ�󴫵ݸ������ͻ���
	if (CharacterEx && GrenadeClass && CharacterEx->GetGrenadeComp())
	{
		const FVector StartingLocation = CharacterEx->GetGrenadeComp()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = CharacterEx;
		SpawnParams.Instigator = CharacterEx;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectileActor>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	//���ӵ�ҩ�� CarriedAmmo -> OnRep && TMap
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		//������󱸵���
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxAmmoAmount);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->Ammo == 0 && EquippedWeapon->WeaponType == WeaponType)
	{
		ReloadWeaponAutomatic();
	}

}

//ֻ���ض�GameMode������
void UCombatComponent::SpawnDefaultWeapon()
{
	AXBlasterGameMode* XBlasterGameMode = Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	//GameModeֻ����Server�Ϸ��ؾ���ֵ���ͻ�����ΪNull
	if (XBlasterGameMode && World && !CharacterEx->IsElimmed() && DefaultWeaponClass)
	{
		AWeaponParent* StartingWeapon = World->SpawnActor<AWeaponParent>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (StartingWeapon)
		{
			EquipWeapon(StartingWeapon);
		}
	}
}

void UCombatComponent::UpdateHUDAmmo()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController && EquippedWeapon)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
		XBlasterPlayerController->SetHUDWeaponAmmo(EquippedWeapon->Ammo);
	}
}

bool UCombatComponent::CouldSwapWeapon()
{
	return (EquippedWeapon && SecondWeapon);
}


