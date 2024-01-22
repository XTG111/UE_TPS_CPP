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

	//��������Ŵ���������
	

	//������������������������ײ
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	//��ʼ����ɫͷ��������ʾ
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	//�������
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	//ս�����ƣ���Ҫͨ�����������Ƹ��ͻ���
	CombatComp->SetIsReplicated(true);

	//�����������
	PropertyComp = CreateDefaultSubobject<UXPropertyComponent>(TEXT("PropertyComp"));
	PropertyComp->SetIsReplicated(true);

	//Lag FrameHistory ��� ֻ�����ڷ�������
	LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComp"));

	//�����Ƿ���µ�boolֵ������UE�Լ������,������������¶׹���
	//	/** Returns true if component can crouch */
	//FORCEINLINE bool CanEverCrouch() const { return NavAgentProps.bCanCrouch; }
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	//Ĭ����ת��û����ת
	TurningInPlace = ETuringInPlace::ETIP_NoTurning;

	//����ˢ��
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	//����ʱ�������
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComp"));

	//����
	AttachGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeComp"));
	//GrenadeSocket
	AttachGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*�����Χ��Box �� ��Ӧ�Ĺ�����*/
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

	//����������Ҫ�ı�����������,ֻ���Ƶ���ǰ�ͻ���
	DOREPLIFETIME_CONDITION(AXCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(AXCharacter, bUnderJump);
	DOREPLIFETIME(AXCharacter, TurningInPlace);
	DOREPLIFETIME(AXCharacter, bDisableGamePlay);
}

//��ʼ������е�ֵ
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

	//��OverHeadWidget����
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
	//������ȴ״̬������ת������
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		return;
	}

	//ֻ�е���ɫȨ�޴���ģ�����Ǳ��ؿ����ǲ�ִ��AimOffset
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		//����ʱ��
		TimeSinceLastMovementReplication += DeltaTime;
		//��ʱ�䳬��һ����ֵ����˵���������ʱ��û�ж�������ôִ��ģ��ĺ���
		//Ȼ������ʱ��
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

	}
}

//��ʼ���κ��޷��ڵ�һ֡��ʼ������
void AXCharacter::PollInit()
{
	if (XBlasterPlayerState == nullptr)
	{
		XBlasterPlayerState = GetPlayerState<AXBlasterPlayerState>();
		if (XBlasterPlayerState)
		{
			XBlasterPlayerState->AddToScore(0.f);
			XBlasterPlayerState->AddToDefeats(0);

			//�����������һ��ʼ��ɫ�ʹ���������ô�����ɻʹ�
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
	//����
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
	//����
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
	//����
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
	//����
	if (bDisableGamePlay) return;
	Super::StopJumping();
	bUnderJump = false;
}

void AXCharacter::ReloadWeapon()
{
	//����
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


//װ������
//ֻ��Ҫ�ڷ���������֤������ڷ������ϵ�Actor
void AXCharacter::EquipWeapon()
{
	if (CombatComp)
	{
		ServerEquipWeapon();
	}
}

//�ڷ������ϵ�Actorִ��
void AXCharacter::ServerEquipWeapon_Implementation()
{
	if (CombatComp)
	{
		CombatComp->NewEquipWeapon();
		//CombatComp->EquipWeapon(OverlappingWeapon);
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

void AXCharacter::DroporDestroyWeapon(AWeaponParent* Weapon)
{
	if (Weapon == nullptr) return;
	//����Ĭ����������ʧ
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
		//����ģ��Ȩ�޿�����ת������
		bRotateRootBone = true;
		bUseControllerRotationYaw = true;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		//AO_Yaw�ĸı����StartingAimRotation��CurrentAimRotation
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		/*AO_Yaw = FMath::FInterpTo(AO_Yaw, DeltaAimRotation.Yaw, DeltaTime, 5.f);*/
		AO_Yaw = DeltaAimRotation.Yaw;

		//��ʼ����ֵ��
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

float AXCharacter::CalculateVelocity()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

////����ԭ��ʱ Yaw��ƫת��90��-90ʱ�޸�״̬
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

	//��ʼת�����ӵ�ǰ�ǶȲ�ֵ��0��ʵ��ת����Ϊת���ķ����Ϊ0
	if (TurningInPlace != ETuringInPlace::ETIP_NoTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAOYaw;
		//���AO_Yaw�ı仯��������״̬�ͳ�ʼ����
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETuringInPlace::ETIP_NoTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

//ģ��ת��
void AXCharacter::SimProxiesturn()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;
	float Speed = CalculateVelocity();
	CalculateAO_Pitch();
	//����Ǳ��ػ��߷��������Ʋ��ܹ�����bRotateRootBOne
	//�����ģ��ת��
	bRotateRootBone = false;
	
	if (Speed > 0.f)
	{
		TurningInPlace = ETuringInPlace::ETIP_NoTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	//ÿ�μ����ת���ֵ���ڸ���ֵ����ô��ִ�в���ת�򶯻�
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

//�����ڷ������ϵı仯
void AXCharacter::Elim(bool bPlayerLeftGame)
{
	if (CombatComp)
	{
		DroporDestroyWeapon(CombatComp->EquippedWeapon);
		DroporDestroyWeapon(CombatComp->SecondWeapon);
	}
	MulticastElim(bPlayerLeftGame);
}

//��Ҫ���������ͻ��˵ı仯
void AXCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	//����Actor����ʱ�����ÿ��������������ӵ���Ϊ��
	if (XBlasterPlayerController)
	{
		XBlasterPlayerController->SetHUDWeaponAmmo(0);
		XBlasterPlayerController->SetHUDCarriedAmmo(0);
	}

	bElimmed = true;
	PlayElimMontage();
	
	//������Ϸ����
	bDisableGamePlay = true;
	GetCharacterMovement()->DisableMovement();
	if (CombatComp)
	{
		GetCombatComp()->IsFired(false);
	}
	//�ı��ɫ��Ĭ�ϲ��ʣ�Ϊ���ܽ�Ч������
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Grow"), 200.0f);
	}
	StartDissolve();

	//�����ƶ�
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (XBlasterPlayerController)
	{
		DisableInput(XBlasterPlayerController);
	}

	//������ײ
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

	//����ɫ�ڿ���ʱ��������Ҫ�ر���׼Ч��
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
	//����������ʱ��
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AXCharacter::ElimTimerFinished, ElimDelay);
}

//����
void AXCharacter::ElimTimerFinished()
{
	//������ɫ
	AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();

	//bLeftGameΪ��ʱ����ʾ��ɫ�뿪����Ϸ��������
	if (XBlasterGameMode  && !bLeftGame)
	{
		XBlasterGameMode->RequestRespawn(this, Controller);
	}
	//�����ؽ�ɫ�뿪����Ϸ����ʼ�㲥���״̬��Ȼ���֪������
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
	GetWorldTimerManager().ClearTimer(ElimTimer);
}

//ElimBot�������ɷ����������ͻ���
void AXCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComp)
	{
		ElimBotComp->DestroyComponent();
	}

	//��ȡ��ǰ����״̬
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

//�ܽ�����
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

//Ϊ����ʾ�����ߵĻʹڵ����пͻ���
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
* �˳���Ϸ���� _Implementation
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