// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "HUD/OverHeadWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "Net/UnrealNetWork.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/XCharacterAnimInstance.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "TimerManager.h"
#include "GameMode/XBlasterGameMode.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "XPlayerState/XBlasterPlayerState.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "GameMode/XBlasterGameMode.h"
#include "BlasterComponent/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameState/XBlasterGameState.h"

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

	//设置相机放大后的清晰度
	

	//消除胶囊体组件对于相机的碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	//初始化角色头顶名字显示
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	//武器组件
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	//战斗控制，需要通过服务器复制给客户端
	CombatComp->SetIsReplicated(true);

	//人物属性组件
	PropertyComp = CreateDefaultSubobject<UXPropertyComponent>(TEXT("PropertyComp"));
	PropertyComp->SetIsReplicated(true);

	//Lag FrameHistory 组件 只存在于服务器上
	LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComp"));

	//控制是否蹲下的bool值，这是UE自己定义的,开启后才能有下蹲功能
	//	/** Returns true if component can crouch */
	//FORCEINLINE bool CanEverCrouch() const { return NavAgentProps.bCanCrouch; }
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//默认旋转是没有旋转
	TurningInPlace = ETuringInPlace::ETIP_NoTurning;

	//网络刷新
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	//构建时间轴组件
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComp"));

	//手榴弹
	AttachGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeComp"));
	//GrenadeSocket
	AttachGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*构造包围盒Box 到 对应的骨骼上*/
	headbox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
	headbox->SetupAttachment(GetMesh(), FName("head"));
	HitBoxCompMap.Add(FName("head"), headbox);

	pelvisbox = CreateDefaultSubobject<UBoxComponent>(TEXT("PelvisBox"));
	pelvisbox->SetupAttachment(GetMesh(), FName("pelvis"));
	HitBoxCompMap.Add(FName("pelvis"), pelvisbox);

	spine_02box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02Box"));
	spine_02box->SetupAttachment(GetMesh(), FName("spine_02"));
	HitBoxCompMap.Add(FName("spine_02"), spine_02box);

	spine_03box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03Box"));
	spine_03box->SetupAttachment(GetMesh(), FName("spine_03"));
	HitBoxCompMap.Add(FName("spine_03"), spine_03box);

	upperarm_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_lBox"));
	upperarm_lbox->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitBoxCompMap.Add(FName("upperarm_l"), upperarm_lbox);

	upperarm_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_rBox"));
	upperarm_rbox->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitBoxCompMap.Add(FName("upperarm_r"), upperarm_rbox);

	lowerarm_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_lboxBox"));
	lowerarm_lbox->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitBoxCompMap.Add(FName("lowerarm_l"), lowerarm_lbox);

	lowerarm_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_rBox"));
	lowerarm_rbox->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitBoxCompMap.Add(FName("lowerarm_r"), lowerarm_rbox);

	hand_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_lBox"));
	hand_lbox->SetupAttachment(GetMesh(), FName("hand_l"));
	HitBoxCompMap.Add(FName("hand_l"), hand_lbox);

	hand_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_rBox"));
	hand_rbox->SetupAttachment(GetMesh(), FName("hand_r"));
	HitBoxCompMap.Add(FName("hand_r"), hand_rbox);

	backpackbox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackBox"));
	backpackbox->SetupAttachment(GetMesh(), FName("backpack"));
	HitBoxCompMap.Add(FName("backpack"), backpackbox);

	blanketbox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlanketBox"));
	blanketbox->SetupAttachment(GetMesh(), FName("backpack"));
	HitBoxCompMap.Add(FName("blacnket"), blanketbox);

	thigh_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_lBox"));
	thigh_lbox->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitBoxCompMap.Add(FName("thigh_l"), thigh_lbox);

	thigh_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_rBox"));
	thigh_rbox->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitBoxCompMap.Add(FName("thigh_r"), thigh_rbox);

	calf_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_lBox"));
	calf_lbox->SetupAttachment(GetMesh(), FName("calf_l"));
	HitBoxCompMap.Add(FName("calf_l"), calf_lbox);

	calf_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_rBox"));
	calf_rbox->SetupAttachment(GetMesh(), FName("calf_r"));
	HitBoxCompMap.Add(FName("calf_r"), calf_rbox);

	foot_lbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_lBox"));
	foot_lbox->SetupAttachment(GetMesh(), FName("foot_l"));
	HitBoxCompMap.Add(FName("foot_l"), foot_lbox);

	foot_rbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_rbox"));
	foot_rbox->SetupAttachment(GetMesh(), FName("foot_r"));
	HitBoxCompMap.Add(FName("foot_r"), foot_rbox);

	for (auto Box : HitBoxCompMap)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AXCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//复制我们需要的变量生命周期,只复制到当前客户端
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AXCharacter, bUnderJump);
	DOREPLIFETIME(AXCharacter, TurningInPlace);
	DOREPLIFETIME(AXCharacter, bDisableGamePlay);
}

//初始化组件中的值
void AXCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComp)
	{
		CombatComp->CharacterEx = this;
	}
	if (PropertyComp)
	{
		PropertyComp->XCharacter = this;
		PropertyComp->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		PropertyComp->SetInitialJump(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComp)
	{
		LagCompensationComp->XCharacter = this;
		if (Controller)
		{
			LagCompensationComp->XCharacterController = Cast<AXBlasterPlayerController>(Controller);
		}
	}
}

void AXCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesturn();
	TimeSinceLastMovementReplication = 0;
}

// Called when the game starts or when spawned
void AXCharacter::BeginPlay()
{
	Super::BeginPlay();

	//给OverHeadWidget声明
	UOverHeadWidget* CharacterHeadWidget = Cast<UOverHeadWidget>(OverHeadWidget->GetUserWidgetObject());
	CharacterHeadWidget->ShowPlayerNetRole(this);
	
	//
	UpdateHUDHealth();
	UpdateHUDShield();

	//OnTakeAnyDamage.AddDynamic(this, &AXCharacter::ReceivedDamage);
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AXCharacter::ReceivedDamage);
	}

	if (AttachGrenade)
	{
		AttachGrenade->SetVisibility(false);
	}
}

// Called every frame
void AXCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void AXCharacter::RotateInPlace(float DeltaTime)
{
	//当在冷却状态不能旋转，考虑
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		return;
	}

	//只有当角色权限大于模拟且是本地控制是才执行AimOffset
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		//更新时间
		TimeSinceLastMovementReplication += DeltaTime;
		//当时间超过一个阈值，那说明我们这段时间没有动作，那么执行模拟的函数
		//然后重置时间
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

	}
}

//初始化任何无法在第一帧初始化的类
void AXCharacter::PollInit()
{
	if (XBlasterPlayerState == nullptr)
	{
		XBlasterPlayerState = GetPlayerState<AXBlasterPlayerState>();
		if (XBlasterPlayerState)
		{
			XBlasterPlayerState->AddToScore(0.f);
			XBlasterPlayerState->AddToDefeats(0);

			//如果重生或者一开始角色就处于领先那么会生成皇冠
			AXBlasterGameState* XBlasterGameState = Cast<AXBlasterGameState>(UGameplayStatics::GetGameState(this));
			if (XBlasterGameState && XBlasterGameState->TopScoringPlayers.Contains(XBlasterPlayerState))
			{
				MulticastGainerTheLead();
			}
		}
	}
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AXCharacter::Fireing);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AXCharacter::ReFired);
	PlayerInputComponent->BindAction("ReloadWeapon", IE_Pressed, this, &AXCharacter::ReloadWeapon);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AXCharacter::Grenade);
	PlayerInputComponent->BindAction("DropWeapon", IE_Pressed, this, &AXCharacter::DropWeapon);
	PlayerInputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AXCharacter::SwapWeapon);
}

void AXCharacter::MoveForward(float value)
{
	//禁用
	if (bDisableGamePlay) return;
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	ForwardValue = value;
	FVector FowardVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
	AddMovementInput(FowardVector, value);
}

void AXCharacter::MoveRight(float value)
{
	//禁用
	if (bDisableGamePlay) return;
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
	//禁用
	if (bDisableGamePlay) return;
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
		bUnderJump = true;
	}
}

void AXCharacter::StopJumping()
{
	//禁用
	if (bDisableGamePlay) return;
	Super::StopJumping();
	bUnderJump = false;
}

void AXCharacter::ReloadWeapon()
{
	//禁用
	if (bDisableGamePlay) return;
	if (CombatComp)
	{
		CombatComp->ReloadWeapon();
	}
}

void AXCharacter::Grenade()
{
	if (CombatComp)
	{
		CombatComp->ThrowGrenade();
	}
}

void AXCharacter::DropWeapon()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->DropEquippedWeapon();
	}
}

void AXCharacter::SwapWeapon()
{
	if (CombatComp && CombatComp->CouldSwapWeapon())
	{
		if (CombatComp->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerSwapWeapon();
		}
		if(!HasAuthority() && CombatComp->CombatState == ECombatState::ECS_Unoccupied)
		{
			PlaySwapMontage();
			CombatComp->CombatState = ECombatState::ECS_SwapingWeapons;
			bFinishedSwap = false;
		}
	}
}

void AXCharacter::ServerSwapWeapon_Implementation()
{
	if (CombatComp && CombatComp->CouldSwapWeapon())
	{
		CombatComp->SwapWeapon();
	}
}


//装备武器
//只需要在服务器上验证，如果在服务器上的Actor
void AXCharacter::EquipWeapon()
{
	if (CombatComp)
	{
		ServerEquipWeapon();
	}
}

//在服务器上的Actor执行
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->NewEquipWeapon();
		//CombatComp->EquipWeapon(OverlappingWeapon);
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

void AXCharacter::DroporDestroyWeapon(AWeaponParent* Weapon)
{
	if (Weapon == nullptr) return;
	//处理默认武器的消失
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Drop();
	}
}

void AXCharacter::AimOffset(float DeltaTime)
{	
	if (CombatComp && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}

	float Speed = CalculateVelocity();
	bool bJump = bUnderJump;
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed > 0.0f || bJump || bIsInAir)
	{
		bRotateRootBone = false;
		bUseControllerRotationYaw = true;
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
	}

	if (Speed == 0.0f && !bJump && !bIsInAir)
	{
		//大于模拟权限可以旋转根骨骼
		bRotateRootBone = true;
		bUseControllerRotationYaw = true;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		//AO_Yaw的改变就是StartingAimRotation到CurrentAimRotation
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		/*AO_Yaw = FMath::FInterpTo(AO_Yaw, DeltaAimRotation.Yaw, DeltaTime, 5.f);*/
		AO_Yaw = DeltaAimRotation.Yaw;

		//初始化插值量
		if (TurningInPlace == ETuringInPlace::ETIP_NoTurning)
		{
			InterpAOYaw = AO_Yaw;
		}

		TurnInPlace(DeltaTime);
	}
		CalculateAO_Pitch();
}

void AXCharacter::CalculateAO_Pitch()
{
	//Pitch的设置
	AO_Pitch = GetBaseAimRotation().Pitch;

	//修正传送过程中的角度压缩
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//客户端-90,0会被压缩为360,270，进行强制转换
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float AXCharacter::CalculateVelocity()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

////当在原地时 Yaw的偏转到90，-90时修改状态
void AXCharacter::TurnInPlace(float DeltaTime)
{
//	UE_LOG(LogTemp, Warning, TEXT("AO_YAW:%f"), AO_Yaw);
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETuringInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETuringInPlace::ETIP_Left;
	}

	//开始转动，从当前角度插值到0，实现转向，因为转向后的方向变为0
	if (TurningInPlace != ETuringInPlace::ETIP_NoTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAOYaw;
		//如果AO_Yaw的变化不大，重置状态和初始朝向
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETuringInPlace::ETIP_NoTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

//模拟转向
void AXCharacter::SimProxiesturn()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	float Speed = CalculateVelocity();
	CalculateAO_Pitch();
	//如果是本地或者服务器控制才能够开启bRotateRootBOne
	//如果是模拟转向
	bRotateRootBone = false;
	
	if (Speed > 0.f)
	{
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//每次计算的转向差值大于给定值，那么将执行播放转向动画
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETuringInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETuringInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		}
		return;
	}
	TurningInPlace = ETuringInPlace::ETIP_NoTurning;
}

void AXCharacter::Fireing()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->IsFired(true);
	}
}

void AXCharacter::ReFired()
{
	if (CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->IsFired(false);
	}
}

bool AXCharacter::GetIsEquippedWeapon()
{
	//if (HasAuthority() && !IsLocallyControlled())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("BWeaponed:%d"), GetIsEquippedWeapon());
	//}
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

void AXCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatComp->bFired && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
		FName SectionName;
		SectionName = bAiming ? FName("AimFire") : FName("FireEquip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AXCharacter::PlayHitReactMontage()
{
	if (CombatComp == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&& HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AXCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance&&ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AXCharacter::PlayReloadMontage()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComp->EquippedWeapon->WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("ReloadRifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("ReloadRocket");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("ReloadPistol");
			break;
		case EWeaponType::EWT_SubMachineGun:
			SectionName = FName("ReloadPistol");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("ReloadShotGun");
			break;
		case EWeaponType::EWT_Snipper:
			SectionName = FName("ReloadSnipper");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("ReloadGrenade");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

//GrenadeMontage
void AXCharacter::PlayGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeMontage)
	{
		AnimInstance->Montage_Play(GrenadeMontage);
	}
}

void AXCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

FVector AXCharacter::GetHitTarget() const
{
	if (CombatComp == nullptr) return FVector();
	return CombatComp->HitTarget;
}

UCameraComponent* AXCharacter::GetFollowCamera() const
{
	return CameraComp;
}

AXBlasterPlayerController* AXCharacter::GetXBlasterPlayerCtr()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(Controller) : XBlasterPlayerController;
	return XBlasterPlayerController;
}

void AXCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((CameraComp->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->WeaponMesh)
		{
			CombatComp->EquippedWeapon->WeaponMesh->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->WeaponMesh)
		{
			CombatComp->EquippedWeapon->WeaponMesh->bOwnerNoSee = false;
		}
	}

}

//void AXCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//}

void AXCharacter::UpdateHUDHealth()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDHealth(PropertyComp->Health, PropertyComp->MAXHealth);
	}
}

void AXCharacter::UpdateHUDShield()
{
	XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(Controller) : XBlasterPlayerController;
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDShield(PropertyComp->Shield, PropertyComp->MAXShield);
	}
}

void AXCharacter::ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	PropertyComp->ReceivedDamage(Damage, InstigatorController);
}

//处理在服务器上的变化
void AXCharacter::Elim(bool bPlayerLeftGame)
{
	if (CombatComp)
	{
		DroporDestroyWeapon(CombatComp->EquippedWeapon);
		DroporDestroyWeapon(CombatComp->SecondWeapon);
	}
	MulticastElim(bPlayerLeftGame);
}

//需要传给其他客户端的变化
void AXCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	//当该Actor死亡时，调用控制器，设置其子弹数为空
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDWeaponAmmo(0);
		XBlasterPlayerController->SetHUDCarriedAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();
	
	//禁用游戏输入
	bDisableGamePlay = true;
	GetCharacterMovement()->DisableMovement();
	if (CombatComp)
	{
		GetCombatComp()->IsFired(false);
	}
	//改变角色的默认材质，为可溶解效果材质
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Grow"), 200.0f);
	}
	StartDissolve();

	//禁用移动
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (XBlasterPlayerController)
	{
		DisableInput(XBlasterPlayerController);
	}

	//禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//ElimBot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	//当角色在开镜时死亡，需要关闭瞄准效果
	bool bHideSnipperScope = IsLocallyControlled()
		&& CombatComp
		&& CombatComp->GetbAiming()
		&& CombatComp->EquippedWeapon
		&& CombatComp->EquippedWeapon->WeaponType == EWeaponType::EWT_Snipper;
	if (bHideSnipperScope)
	{
		ShowSnipperScope(false);
	}

	if (CrownComp)
	{
		CrownComp->DestroyComponent();
	}
	//开启重生计时器
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AXCharacter::ElimTimerFinished, ElimDelay);
}

//重生
void AXCharacter::ElimTimerFinished()
{
	//重生角色
	AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();

	//bLeftGame为真时，表示角色离开了游戏不再重生
	if (XBlasterGameMode  && !bLeftGame)
	{
		XBlasterGameMode->RequestRespawn(this, Controller);
	}
	//当本地角色离开了游戏，开始广播这个状态，然后告知主界面
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
	GetWorldTimerManager().ClearTimer(ElimTimer);
}

//ElimBot的销毁由服务器传给客户端
void AXCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComp)
	{
		ElimBotComp->DestroyComponent();
	}

	//获取当前比赛状态
	AXBlasterGameMode* XBlasterGameMode = Cast<AXBlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = XBlasterGameMode && XBlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComp && CombatComp->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComp->EquippedWeapon->Destroy();
	}
}

UXPropertyComponent* AXCharacter::GetPropertyComp()
{
	return PropertyComp;
}

//溶解死亡
void AXCharacter::UpdateDissolveMaterial(float Dissolve)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), Dissolve);
	}
}
void AXCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AXCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

ECombatState AXCharacter::GetCombateState() const
{
	if (CombatComp == nullptr) return ECombatState::ECS_MAX;
	return CombatComp->CombatState;
}

bool AXCharacter::IsLocallyReloading()
{
	if (CombatComp == nullptr) return false;
	return CombatComp->bLocallyReloading;
}

//为了显示领先者的皇冠到所有客户端
void AXCharacter::MulticastGainerTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComp == nullptr)
	{
		CrownComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComp)
	{
		CrownComp->Activate();
	}

}

void AXCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComp)
	{
		CrownComp->DestroyComponent();
	}
}

/*
* 退出游戏处理 _Implementation
*/
void AXCharacter::ServerLeaveGame_Implementation()
{
	AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();
	XBlasterPlayerState = XBlasterPlayerState == nullptr ? GetPlayerState<AXBlasterPlayerState>() : XBlasterPlayerState;
	if (XBlasterGameMode && XBlasterPlayerState)
	{
		XBlasterGameMode->PlayerLeftGame(XBlasterPlayerState);
	}
}