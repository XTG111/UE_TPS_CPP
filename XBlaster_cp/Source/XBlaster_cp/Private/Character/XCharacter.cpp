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
#include "Components/CapsuleComponent.h"

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

	//������������������������ײ
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	//��ʼ����ɫͷ��������ʾ
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	//�������
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	//ս�����ƣ���Ҫͨ�����������Ƹ��ͻ���
	CombatComp->SetIsReplicated(true);

	//�����Ƿ���µ�boolֵ������UE�Լ������,������������¶׹���
	//	/** Returns true if component can crouch */
	//FORCEINLINE bool CanEverCrouch() const { return NavAgentProps.bCanCrouch; }
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//Ĭ����ת��û����ת
	TurningInPlace = ETuringInPlace::ETIP_NoTurning;
}

void AXCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//����������Ҫ�ı�����������,ֻ���Ƶ���ǰ�ͻ���
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AXCharacter, bUnderJump);
	DOREPLIFETIME(AXCharacter, AO_Yaw);
	DOREPLIFETIME(AXCharacter, TurningInPlace);
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
	AimOffset(DeltaTime);
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
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AXCharacter::CrouchMode);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AXCharacter::RelaxToAimMode);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AXCharacter::AimToRelaxMode);
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

void AXCharacter::CrouchMode()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AXCharacter::RelaxToAimMode()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(true);
	}
}

void AXCharacter::AimToRelaxMode()
{
	if (CombatComp)
	{
		CombatComp->SetAiming(false);
	}
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
		ServerEquipWeapon();
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

void AXCharacter::AimOffset(float DeltaTime)
{	AOYawTrans(DeltaTime);

	//Pitch������
	AO_Pitch = GetBaseAimRotation().Pitch;

	//�������͹����еĽǶ�ѹ��
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//�ͻ���-90,0�ᱻѹ��Ϊ360,270������ǿ��ת��
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AXCharacter::AOYawTrans_Implementation(float DeltaTime)
{
	if (CombatComp && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bJump = bUnderJump;
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed > 0.0f || bJump || bIsInAir)
	{
		bUseControllerRotationYaw = true;
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
	}

	if (Speed == 0.0f && !bJump && !bIsInAir)
	{
		bUseControllerRotationYaw = false;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		//AO_Yaw�ĸı����StartingAimRotation��CurrentAimRotation
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		/*AO_Yaw = FMath::FInterpTo(AO_Yaw, DeltaAimRotation.Yaw, DeltaTime, 5.f);*/
		AO_Yaw = DeltaAimRotation.Yaw;

		//��ʼ����ֵ��
		//if (TurningInPlace == ETuringInPlace::ETIP_NoTurning)
		//{
		//	InterpAOYaw = AO_Yaw;
		//}

		//TurnInPlace(DeltaTime);
	}
}

////����ԭ��ʱ Yaw��ƫת��90��-90ʱ�޸�״̬
//void AXCharacter::TurnInPlace(float DeltaTime)
//{
//	if (AO_Yaw > 90.f)
//	{
//		TurningInPlace = ETuringInPlace::ETIP_Right;
//	}
//	else if (AO_Yaw < -90.f)
//	{
//		TurningInPlace = ETuringInPlace::ETIP_Left;
//	}
//
//	//��ʼת�����ӵ�ǰ�ǶȲ�ֵ��0��ʵ��ת����Ϊת���ķ����Ϊ0
//	if (TurningInPlace != ETuringInPlace::ETIP_NoTurning)
//	{
//		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.f, DeltaTime, 5.f);
//		AO_Yaw = InterpAOYaw;
//		//���AO_Yaw�ı仯��������״̬�ͳ�ʼ����
//		if (FMath::Abs(AO_Yaw) < 15.f)
//		{
//			TurningInPlace = ETuringInPlace::ETIP_NoTurning;
//			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
//		}
//	}
//}

bool AXCharacter::GetIsEquippedWeapon()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool AXCharacter::GetIsAiming()
{
	return (CombatComp && CombatComp->bUnderAiming);
}

float AXCharacter::GetAOYawToAnim() const
{
	return AO_Yaw;
}

float AXCharacter::GetAOPitchToAnim() const
{
	return AO_Pitch;
}

AWeaponParent* AXCharacter::GetEquippedWeapon()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		return CombatComp->EquippedWeapon;
	}
	return nullptr;
}

ETuringInPlace AXCharacter::GetTurninigInPlace() const
{
	return TurningInPlace;
}



