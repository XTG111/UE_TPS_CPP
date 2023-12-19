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

	//消除胶囊体组件对于相机的碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	//初始化角色头顶名字显示
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	//武器组件
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	//战斗控制，需要通过服务器复制给客户端
	CombatComp->SetIsReplicated(true);

	//控制是否蹲下的bool值，这是UE自己定义的,开启后才能有下蹲功能
	//	/** Returns true if component can crouch */
	//FORCEINLINE bool CanEverCrouch() const { return NavAgentProps.bCanCrouch; }
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AXCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//复制我们需要的变量生命周期,只复制到当前客户端
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AXCharacter, bUnderJump);
}

//初始化组件中的值
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

	//给OverHeadWidget声明
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


//装备武器
//只需要在服务器上验证，如果在服务器上的Actor
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

//在客户端上的Actor执行
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}

//客户端调用
//LastWeapon代表了上一次重叠事件的Actor，对于现在来说，如果我们离开了检测去，LastWeapon就不为空，这时候就要把它的widget设置为false
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

//服务器调用
void AXCharacter::SetOverlappingWeapon(AWeaponParent* Weapon)
{
	//对于服务器上的角色操作，我们在对OverlappingWeapon赋值之前检测，它是否有效，如果有效那么说明之前产生过，而当球体的EndOverlap函数也调用的是这个函数，所以当我们离开时
	//WOverlappingWeapon一定不为空，这时候直接设置false即可
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}

	OverlappingWeapon = Weapon;

	//写的所有逻辑都是针对服务器而言，通过复制将一些属性复制到客户端，利用了Rep_Notify使得UI显示只有客户端能看见
	//为了使得服务器上的角色也能够看见，那么就要单独判断
	//通过IsLocallyControlled()来判断当前是否是服务器上的本地操作
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

void AXCharacter::AimOffset(float DeltaTime)
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
		bUseControllerRotationYaw = false;
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
	}

	if (Speed == 0.0f && !bJump && !bIsInAir)
	{
		bUseControllerRotationYaw = false;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		//AO_Yaw的改变就是StartingAimRotation到CurrentAimRotation
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = FMath::FInterpTo(AO_Yaw, DeltaAimRotation.Yaw, DeltaTime, 5.f);
	}

	//Pitch的设置
	AO_Pitch = GetBaseAimRotation().Pitch;
}

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




