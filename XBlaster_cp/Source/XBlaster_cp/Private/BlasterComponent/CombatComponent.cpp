// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/CombatComponent.h"
#include "Weapon/WeaponParent.h"
#include "Character/XCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetWork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...

	BaseWalkSpeed = 350.f;
	AimWalkSpeed = 100.f;
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bUnderAiming);
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