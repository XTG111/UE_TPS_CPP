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
		//获得准星位置
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		//绘制准星
		SetHUDCrossHairs(DeltaTime);

		//更改视角
		InterpFOV(DeltaTime);
	}

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondWeapon);
	DOREPLIFETIME(UCombatComponent, bUnderAiming);
	//只对Owner客户端复制
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
	//初始化角色持有枪械的备弹量
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
	//为了避免网络延迟，导致动画的延迟
	bUnderAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (CharacterEx)
	{
		CharacterEx->GetCharacterMovement()->MaxWalkSpeed = bUnderAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	//只在本地运行
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
	//丢枪后更新UI
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
	//当拥有武器丢下
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
}

//附加到右手
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (CharacterEx == nullptr || CharacterEx->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	//附加
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
	//附加
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, CharacterEx->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackPackage(AActor* ActorToAttach)
{
	if (CharacterEx == nullptr || CharacterEx->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = CharacterEx->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	//附加
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, CharacterEx->GetMesh());
	}
}

//备弹数的UI
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	//设置CarriedAmmo
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
	}
	//绘制备弹的UI
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

//播放装备武器的音效
void UCombatComponent::PlayEquipWeaponSound(AWeaponParent* WeaponToEquip)
{
	//播放装备武器时的音效
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
	//如果此时不是空闲状态那么不进行装备武器
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	//装备第二把武器
	if (EquippedWeapon != nullptr && SecondWeapon == nullptr)
	{
		EquipSecondWeapon(WeaponToEquip);
	}
	else
	{
		//装备第一把武器
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	//拾取武器后切换控制
	CharacterEx->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterEx->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponParent* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//丢弃已有的武器
	ChangeEquippedWeapon();
	//使用骨骼插槽设置武器位置
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//附加到右手
	AttachActorToRightHand(EquippedWeapon);
	//SetOwner中的形参Owner这个函数，UE已经默认了进行属性复制
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	EquippedWeapon->SetOwner(CharacterEx);
	EquippedWeapon->SetHUDAmmo();
	//更新备弹数及UI
	UpdateCarriedAmmo();
	//装备音效
	PlayEquipWeaponSound(WeaponToEquip);
	//自动换弹
	ReloadWeaponAutomatic();
}

void UCombatComponent::EquipSecondWeapon(AWeaponParent* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	//使用骨骼插槽设置武器位置
	SecondWeapon = WeaponToEquip;
	SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
	//附加到右手
	AttachActorToBackPackage(WeaponToEquip);
	//SetOwner中的形参Owner这个函数，UE已经默认了进行属性复制
	//UPROPERTY(ReplicatedUsing = OnRep_Owner)
	SecondWeapon->SetOwner(CharacterEx);
	//装备音效
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
	//使用面扫
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
		//武器状态的设置和附加到手上的操作不能判断先后的，所以在客户端上也进行修改保证正确性
		//拾取武器后切换控制
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		//播放装备武器时的音效
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
		//武器状态的设置和附加到手上的操作不能判断先后的，所以在客户端上也进行修改保证正确性
		//拾取武器后切换控制
		SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
		AttachActorToBackPackage(SecondWeapon);
		//播放装备武器时的音效
		PlayEquipWeaponSound(SecondWeapon);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || CharacterEx == nullptr) return;
	CharacterEx->GetWorldTimerManager().SetTimer(FireTime, this, &UCombatComponent::FireTimeFinished, EquippedWeapon->FireDelay);
}
//当定时器结束再调用ControlFire相当于一个循环
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

//同步备弹数
void UCombatComponent::OnRep_CarriedAmmo()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
		//XBlasterPlayerController->SetHUDWeaponAmmo(EquippedWeapon->Ammo);
	}
	//霰弹枪的跳转换弹同步到客户端
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

//Client Use 通过RPC修改的CombatState播放动画
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
	case ECombatState::ECS_ThrowingGrenade: //OnRepNotify服务器到其他客户端
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
		//在动画播放结束之后更新
		UpdateAmmoAndCarriedAmmo();
	}
}

void UCombatComponent::UpdateAmmoAndCarriedAmmo()
{
	if (CharacterEx == nullptr && EquippedWeapon == nullptr) return;

	//补充子弹
	int32 ReloadAmount = AmmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		//更新备弹数以及UI显示的备弹数
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

//霰弹枪换弹调用，被动画通知调用
void UCombatComponent::ShotGunUpdateAmmoAndCarriedAmmo()
{
	if (CharacterEx == nullptr && EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		//更新备弹数以及UI显示的备弹数
		CarriedAmmoMap[EquippedWeapon->WeaponType] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->WeaponType];
	}

	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(CharacterEx->Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(-1);

	//当增加了一发子弹就可以开火了
	bCanFire = true;
	//MaxAmmo 5
	//当换弹到maxammo 或者 没有备弹那么跳转
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
			//播放蒙太奇动画以及伤害判定
			ServerFire(bPressed, HitTarget);
			if (EquippedWeapon)
			{
				CrosshairShootingFactor = 0.75f;
			}
		}
		//发射子弹
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

	//对于霰弹枪的单独开枪判断
	if (EquippedWeapon->Ammo > 0 && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->WeaponType == EWeaponType::EWT_ShotGun)
	{
		return true;
	}

	return EquippedWeapon->Ammo > 0 && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

//执行开枪
void UCombatComponent::IsFired(bool bPressed)
{
	bFired = bPressed;
	ControlFire(bFired);
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
	// 
	// 对于霰弹枪
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

	//开火粒子特效的播放
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

		//当命中对象继承了InteractWithCrosshairInterface接口时，改变准星颜色
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

			//瞄准时准星变换
			if (bUnderAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, Deltatime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, Deltatime, 30.f);
			}

			//射击后应该插值回0
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, Deltatime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			XBlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

int32 UCombatComponent::AmmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	//计算需要补充多少弹量
	int32 RoomInMag = EquippedWeapon->MaxAmmo - EquippedWeapon->Ammo;
	//获取当前的备弹数
	if (CarriedAmmoMap.Contains(EquippedWeapon->WeaponType))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->WeaponType];
		//计算待补充和备弹数的最小值，为实际可以增加的子弹数
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

//本地客户端的调用
void UCombatComponent::ThrowGrenade()
{
	//避免当前手雷弹数量为0
	if (GrenadeAmount == 0) return;

	//如果当前状态不是空闲，就不能继续执行，避免多次按键
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	//修改CombatState这样通RepNotify服务器会传递给客户端，还需要ServerRPC将客户端传递给服务器
	CombatState = ECombatState::ECS_ThrowingGrenade;

	//如果当前在本地客户端，那么避免延迟可以先播放mongtage动画
	if (CharacterEx)
	{
		CharacterEx->PlayGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//调用ServerRPC使得服务器上执行 !CharacterEx->HasAuthority()避免重复调用
	if (CharacterEx && !CharacterEx->HasAuthority())
	{
		ServerThrowGrenade();
	}

	//由于如果不在服务器上不会执行ServerThrowGrenade();，所以要单独调用UI设置
	if (CharacterEx && CharacterEx->HasAuthority())
	{
		//在服务器上处理手雷数量减少
		GrenadeAmount = FMath::Clamp(GrenadeAmount - 1, 0, MaxGrenade);
		UpdateHUDGrenade();
	}
}

//服务器上的实现
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	//在服务器上也检测，避免作弊
	if (GrenadeAmount == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (CharacterEx)
	{
		CharacterEx->PlayGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//在服务器上处理手雷数量减少
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

//交换武器
void UCombatComponent::SwapWeapon()
{
	AWeaponParent* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondWeapon;
	SecondWeapon = TempWeapon;

	//附加
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	//Ammo是没有OnRep的所以我们通过EquippedWeapon来传递HUD的更新
	EquippedWeapon->SetHUDAmmo();
	//备用弹量是可复制的，所以利用其可以更新客户端的HUD
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondWeapon->SetWeaponState(EWeaponState::EWS_Second);
	AttachActorToBackPackage(SecondWeapon);
}

//插值设置视角
void UCombatComponent::InterpFOV(float Deltatime)
{
	if (EquippedWeapon == nullptr) return;

	//调整武器的ZoomFOV和ZoomInterSpeed
	if (bUnderAiming)
	{
		//当我们正在瞄准，视角变换将是武器设置的视野和变化速度
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->ZoomFOV, Deltatime, EquippedWeapon->ZoomInterpSpeed);
	}
	else
	{
		//当我们取消瞄准，视角将变为默认并且是我们默认设置的变化速度
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

//生成手雷粒子组件
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	//只在服务器上生成，然后传递给其他客户端
	if (CharacterEx && CharacterEx->IsLocallyControlled())
	{
		ServerLauncherGrenade(HitTarget);
	}
}

//Server use
void UCombatComponent::ServerLauncherGrenade_Implementation(const FVector_NetQuantize& Target)
{
	//只在服务器上生成，然后传递给其他客户端
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
	//增加弹药量 CarriedAmmo -> OnRep && TMap
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		//控制最大备弹数
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxAmmoAmount);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->Ammo == 0 && EquippedWeapon->WeaponType == WeaponType)
	{
		ReloadWeaponAutomatic();
	}

}

//只在特定GameMode下生成
void UCombatComponent::SpawnDefaultWeapon()
{
	AXBlasterGameMode* XBlasterGameMode = Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	//GameMode只会在Server上返回具体值，客户端上为Null
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


