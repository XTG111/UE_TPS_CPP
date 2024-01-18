// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponParent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/XCharacter.h"
#include "Net/UnrealNetWork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/BulletShellActor.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Character/XCharacter.h"
#include "BlasterComponent/CombatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeaponParent::AWeaponParent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//�����пͻ���������Ϊ����
	bReplicates = true;

	//���������˶�Ҳ����Ϊ����
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	//�����赲������
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//�Խ�ɫ���ú��ԣ�ʹ�ý�ɫ���ᱻ���赲
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//��λ�����ϵ�ʱ�򣬲�������ײ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//������ɫ
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	//�������ܷ�ʰȡ���ڷ������Ͻ��У������ڿͻ��˿��Խ�ͨ������Ϊ����
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	//�ͻ����ϵĿ���
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//ʰȡ��ʾ
	PickUpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidgetComp"));
	PickUpWidgetComp->SetupAttachment(RootComponent);
}

void AWeaponParent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponParent, WeaponState);
}


// Called when the game starts or when spawned
void AWeaponParent::BeginPlay()
{
	Super::BeginPlay();
	PickUpWidgetComp->SetVisibility(false);
	//��Ŀǰ�����Ƿ�����ʱ����������ײ�����Ұ��ص��¼�����LocleRole
	//if (GetLocalRole() == ENetRole::ROLE_Authority)
	//����ɫ����ײ�����ص�ʱ�����¼����������Ի������ýӿ����߼��
	SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//�ڷ������ϴ����ص��¼�
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AWeaponParent::OnSphereBeginOverlap);
	SphereComp->OnComponentEndOverlap.AddDynamic(this, &AWeaponParent::OnSphereEndOverlap);
}

// Called every frame
void AWeaponParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//ͨ�����ƣ��ͻ��˵���
void AWeaponParent::OnRep_WeponState()
{
	OnWeaponStateSet();
}

//����������
void AWeaponParent::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeaponParent::ShowPickUpWidget(bool bShowWidget)
{
	if (PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(bShowWidget);
	}
	
}

void AWeaponParent::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	//�����׿� Socket AmmoEject
	if (BulletShellClass)
	{
		//��ȡ���
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ABulletShellActor>(BulletShellClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	//��ǹ���ø����ӵ�ֻ�ڷ������Ͻ���
	//if (HasAuthority()) 
	//{
	//	SpendRound();
	//}
	//�ͻ���Ԥ��
	SpendRound();
}

void AWeaponParent::SetHUDAmmo()
{
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}

}


void AWeaponParent::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);
	SetHUDAmmo();
	//�������ϵ���CLientRPC��ͻ��˴���Ȩ��ֵ
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	//���ǿͻ����Ǽ�¼���ͼ�¼�Ĵ���
	else
	{
		Sequence++;
	}
}

void AWeaponParent::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	//����������Ȩ��ֵ��ֵ���ͻ���
	Ammo = ServerAmmo;
	//�Ѿ�������һ����������
	Sequence--;
	//У���������ӵ������ĵģ��ͻ���Ԥ����ӵ��϶��ȷ��������������� ���sequence
	Ammo -= Sequence;
	SetHUDAmmo();
}


void AWeaponParent::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MaxAmmo);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientAddAmmo(AmmoToAdd);
	}
}

void AWeaponParent::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MaxAmmo);
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter && XCharacter->GetCombatComp() && IsFull())
	{
		XCharacter->GetCombatComp()->JumpToShotGunEnd();
	}
	SetHUDAmmo();
}

//��дOnwer������ͻ��˼�ʱ��Ӧ
void AWeaponParent::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		XCharacter = nullptr;
		XBlasterPlayerController = nullptr;
	}
	else
	{
		XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
		//ֻ�����������ǲŸ���UI
		if (XCharacter && XCharacter->GetEquippedWeapon() && XCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeaponParent::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	FVector ForwardVector = UKismetMathLibrary::GetForwardVector(GetActorRotation());
	FVector ImpulseVec = ForwardVector * 165.f;
	//����������Ӿ�����
	WeaponMesh->AddImpulse(ImpulseVec);
	SetOwner(nullptr);
	XCharacter = nullptr;
	XBlasterPlayerController = nullptr;
}

bool AWeaponParent::IsFull()
{
	return Ammo == MaxAmmo;
}

void AWeaponParent::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

//ʹ�ýӿ�ͨ��
void AWeaponParent::FPickObject_Implementation(APawn* InstigatorPawn)
{
	XCharacter = Cast<AXCharacter>(InstigatorPawn);
	if (XCharacter && XCharacter->GetCombatComp())
	{
		XCharacter->GetCombatComp()->EquipWeapon(this);
	}
}

FVector AWeaponParent::TraceEndWithScatter(const FVector& HitTarget)
{

	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//��������Ҫ�ڱ��ؼ���ɢ������ֱ�Ӵ���·���Ŀ�ʼλ��
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	//��ǹ�ڳ���
	const FVector TraceStart = SockertTransform.GetLocation();
	//�����ָ��Ŀ��ķ�������
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();

	//ɢ��Բ��λ��
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);

	//�����������1��
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;

	FVector ToEndLoc = EndLoc - TraceStart;
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);

	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()), FColor::Cyan, true);
	return FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size());
}

void AWeaponParent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		XCharacter->SetOverlappingWeapon(this);
	}
}

void AWeaponParent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		XCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeaponParent::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		HandleOnEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		HandleOnDropped();
		break;
	case EWeaponState::EWS_Second:
		HandleOnSecond();
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeaponParent::HandleOnEquipped()
{
	//��װ����ʱӦ���ر�UI��������ײ
	ShowPickUpWidget(false);
	//����������ײ
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//���װ����ʹSMG������ģ������
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
}

void AWeaponParent::HandleOnDropped()
{
	if (HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//���û�Ĭ��״̬
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AWeaponParent::HandleOnSecond()
{
	//��װ����ʱӦ���ر�UI��������ײ
	ShowPickUpWidget(false);
	//����������ײ
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//���װ����ʹSMG������ģ������
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();
}

