// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Character/XCharacter.h"

AFlag::AFlag()
{
	FlagSMComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagSMComp"));
	SetRootComponent(FlagSMComp);
	SphereComp->SetupAttachment(FlagSMComp);
	PickUpWidgetComp->SetupAttachment(FlagSMComp);
	FlagSMComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagSMComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlag::Drop()
{
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	//���þ�����
s	if (XCharacter && XCharacter->GetCombatComp())
	{
		XCharacter->SetOverlappingWeapon(nullptr);
		XCharacter->GetCombatComp()->SetHoldingTheFlag(false);
		XCharacter->UnCrouch();
	}

	if (!HasAuthority()) return;
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagSMComp->DetachFromComponent(DetachRules);

	FVector ForwardVector = UKismetMathLibrary::GetForwardVector(GetActorRotation());
	FVector ImpulseVec = ForwardVector * 165.f;
	//����������Ӿ�����
	FlagSMComp->AddImpulse(ImpulseVec);

	SetOwner(nullptr);
	XCharacter = nullptr;
	XBlasterPlayerController = nullptr;
}

void AFlag::ResetFlag()
{
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	//���þ�����
	if (XCharacter->GetCombatComp())
	{
		XCharacter->SetOverlappingWeapon(nullptr);
		XCharacter->GetCombatComp()->SetHoldingTheFlag(false);
		XCharacter->UnCrouch();
	}

	if (!HasAuthority()) return;
	SetWeaponState(EWeaponState::EWS_Initial);
	SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	FlagSMComp->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	XCharacter = nullptr;
	XBlasterPlayerController = nullptr;
	SetActorTransform(InitialTransform);
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	InitialTransform = GetActorTransform();
}

void AFlag::HandleOnEquipped()
{
	//��װ����ʱӦ���ر�UI��������ײ
	ShowPickUpWidget(false);
	//����������ײ
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagSMComp->SetSimulatePhysics(false);
	FlagSMComp->SetEnableGravity(false);
	FlagSMComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagSMComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
}
void AFlag::HandleOnDropped()
{
	if (HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagSMComp->SetSimulatePhysics(true);
	FlagSMComp->SetEnableGravity(true);
	FlagSMComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//���û�Ĭ��״̬
	FlagSMComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FlagSMComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagSMComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

