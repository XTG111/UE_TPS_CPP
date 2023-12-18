// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/OverHeadWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "Net/UnrealNetWork.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"


// Sets default values
AXCharacter::AXCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetMesh());
	SpringArmComp->TargetArmLength = 600.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp,USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	//��ʼ����ɫͷ��������ʾ
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	//�������
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	//ս�����ƣ���Ҫͨ�����������Ƹ��ͻ���
	CombatComp->SetIsReplicated(true);
}

void AXCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//����������Ҫ�ı�����������,ֻ���Ƶ���ǰ�ͻ���
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
}

//��ʼ������е�ֵ
void AXCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComp)
	{
		CombatComp->CharacterEx = this;
	}
}

// Called when the game starts or when spawned
void AXCharacter::BeginPlay()
{
	Super::BeginPlay();

	//��OverHeadWidget����
	UOverHeadWidget* CharacterHeadWidget = Cast<UOverHeadWidget>(OverHeadWidget->GetUserWidgetObject());
	CharacterHeadWidget->ShowPlayerNetRole(this);
	
}

// Called every frame
void AXCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AXCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AXCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AXCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AXCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AXCharacter::Turn);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AXCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AXCharacter::StopJumping);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AXCharacter::EquipWeapon);
}

void AXCharacter::MoveForward(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	ForwardValue = value;
	FVector FowardVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
	AddMovementInput(FowardVector, value);
}

void AXCharacter::MoveRight(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	RightValue = value;
	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	AddMovementInput(RightVector, value);
}

void AXCharacter::LookUp(float value)
{
	AddControllerYawInput(value);
}

void AXCharacter::Turn(float value)
{
	AddControllerPitchInput(value);
}

void AXCharacter::Jump()
{
	Super::Jump();
	bUnderJump = true;
}

void AXCharacter::StopJumping()
{
	Super::StopJumping();
	bUnderJump = false;
}


//װ������
//ֻ��Ҫ�ڷ���������֤������ڷ������ϵ�Actor
void AXCharacter::EquipWeapon()
{
	if (CombatComp)
	{
		if (HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipWeapon();
		}
	}
}
//�ڿͻ����ϵ�Actorִ��
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}

//�ͻ��˵���
//LastWeapon��������һ���ص��¼���Actor������������˵����������뿪�˼��ȥ��LastWeapon�Ͳ�Ϊ�գ���ʱ���Ҫ������widget����Ϊfalse
void AXCharacter::OnRep_OverlappingWeapon(AWeaponParent* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}


//����������
void AXCharacter::SetOverlappingWeapon(AWeaponParent* Weapon)
{
	//���ڷ������ϵĽ�ɫ�����������ڶ�OverlappingWeapon��ֵ֮ǰ��⣬���Ƿ���Ч�������Ч��ô˵��֮ǰ�����������������EndOverlap����Ҳ���õ���������������Ե������뿪ʱ
	//WOverlappingWeaponһ����Ϊ�գ���ʱ��ֱ������false����
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}

	OverlappingWeapon = Weapon;

	//д�������߼�������Է��������ԣ�ͨ�����ƽ�һЩ���Ը��Ƶ��ͻ��ˣ�������Rep_Notifyʹ��UI��ʾֻ�пͻ����ܿ���
	//Ϊ��ʹ�÷������ϵĽ�ɫҲ�ܹ���������ô��Ҫ�����ж�
	//ͨ��IsLocallyControlled()���жϵ�ǰ�Ƿ��Ƿ������ϵı��ز���
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}




