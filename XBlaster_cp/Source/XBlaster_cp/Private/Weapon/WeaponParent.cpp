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
	DOREPLIFETIME(AWeaponParent, Ammo);
}


// Called when the game starts or when spawned
void AWeaponParent::BeginPlay()
{
	Super::BeginPlay();
	PickUpWidgetComp->SetVisibility(false);
	//��Ŀǰ�����Ƿ�����ʱ����������ײ�����Ұ��ص��¼�����LocleRole
	//if (GetLocalRole() == ENetRole::ROLE_Authority)
	if(HasAuthority())
	{
		//����ɫ����ײ�����ص�ʱ�����¼����������Ի������ýӿ����߼��
		SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//�ڷ������ϴ����ص��¼�
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AWeaponParent::OnSphereBeginOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &AWeaponParent::OnSphereEndOverlap);
	}
	
}

// Called every frame
void AWeaponParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//ͨ�����ƣ��ͻ��˵���
void AWeaponParent::OnRep_WeponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

//����������
void AWeaponParent::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		//��װ����ʱӦ���ر�UI��������ײ
		ShowPickUpWidget(false);
		//����������ײ
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
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
	//��ǹ����
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

void AWeaponParent::OnRep_Ammo()
{
	SetHUDAmmo();
}

void AWeaponParent::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);
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
		SetHUDAmmo();
	}
}

void AWeaponParent::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	XCharacter = nullptr;
	XBlasterPlayerController = nullptr;
}

void AWeaponParent::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MaxAmmo);
	SetHUDAmmo();
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

