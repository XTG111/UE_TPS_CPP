// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponParent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/XCharacter.h"
#include "Net/UnrealNetWork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AWeaponParent::AWeaponParent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//在所有客户端武器都为副本
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	//设置阻挡方便检测
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//对角色设置忽略，使得角色不会被其阻挡
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//当位于手上的时候，不开启碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//球体检测能否拾取，在服务器上进行，所以在客户端可以将通道设置为忽略
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	//客户端上的控制
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//拾取提示
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
	//当目前机器是服务器时启用球体碰撞，并且绑定重叠事件利用LocleRole
	//if (GetLocalRole() == ENetRole::ROLE_Authority)
	if(HasAuthority())
	{
		//当角色与碰撞球体重叠时产生事件，后续可以换成利用接口射线检测
		SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//在服务器上处理重叠事件
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AWeaponParent::OnSphereBeginOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &AWeaponParent::OnSphereEndOverlap);
	}
	
}

// Called every frame
void AWeaponParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//通过复制，客户端调用
void AWeaponParent::OnRep_WeponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		
		//SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

//服务器调用
void AWeaponParent::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		//当装备上时应当关闭UI和球体碰撞
		ShowPickUpWidget(false);
		//禁用球体碰撞
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
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
		WeaponMesh->PlayAnimation(FireAnimation, true);
	}
}

void AWeaponParent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AXCharacter* XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		XCharacter->SetOverlappingWeapon(this);
	}
}

void AWeaponParent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	AXCharacter* XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		XCharacter->SetOverlappingWeapon(nullptr);
	}
}

