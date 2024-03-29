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
	//在所有客户端武器都为副本
	bReplicates = true;

	//将武器的运动也设置为复制
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	//设置阻挡方便检测
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//对角色设置忽略，使得角色不会被其阻挡
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//当位于手上的时候，不开启碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//设置颜色
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

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
	DOREPLIFETIME_CONDITION(AWeaponParent, bUseServerSideRewide,COND_OwnerOnly);
}


// Called when the game starts or when spawned
void AWeaponParent::BeginPlay()
{
	Super::BeginPlay();
	PickUpWidgetComp->SetVisibility(false);
	//当目前机器是服务器时启用球体碰撞，并且绑定重叠事件利用LocleRole
	//if (GetLocalRole() == ENetRole::ROLE_Authority)
	//当角色与碰撞球体重叠时产生事件，后续可以换成利用接口射线检测
	SphereComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//在服务器上处理重叠事件
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AWeaponParent::OnSphereBeginOverlap);
	SphereComp->OnComponentEndOverlap.AddDynamic(this, &AWeaponParent::OnSphereEndOverlap);
}

// Called every frame
void AWeaponParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//通过复制，客户端调用
void AWeaponParent::OnRep_WeponState()
{
	OnWeaponStateSet();
}

//服务器调用
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
	//生成抛壳 Socket AmmoEject
	if (BulletShellClass)
	{
		//获取插槽
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
	//开枪调用更新子弹只在服务器上进行
	//if (HasAuthority()) 
	//{
	//	SpendRound();
	//}
	//客户端预测
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
	//服务器上调用CLientRPC向客户端传递权威值
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	//当是客户端是记录发送记录的次数
	else
	{
		Sequence++;
	}
}

void AWeaponParent::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	//将服务器的权威值赋值给客户端
	Ammo = ServerAmmo;
	//已经处理了一个发送请求
	Sequence--;
	//校正，由于子弹是消耗的，客户端预测的子弹肯定比服务器传回来的少 相差sequence
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

//重写Onwer，方便客户端及时响应
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
		//只有是主武器是才更新UI
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
	//丢弃武器添加径向力
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

//使用接口通信
void AWeaponParent::FPickObject_Implementation(APawn* InstigatorPawn)
{
	XCharacter = Cast<AXCharacter>(InstigatorPawn);
	if (XCharacter && XCharacter->GetCombatComp())
	{
		if (WeaponType == EWeaponType::EWT_Flag && XCharacter->GetTeam() == TeamType) return;
		XCharacter->GetCombatComp()->EquipWeapon(this);
	}
}

FVector AWeaponParent::TraceEndWithScatter(const FVector& HitTarget)
{

	//获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//由于我们要在本地计算散布所以直接传入路径的开始位置
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	//从枪口出发
	const FVector TraceStart = SockertTransform.GetLocation();
	//从起点指向目标的方向向量
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();

	//散布圆心位置
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);

	//随机生成球内1点
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
		if (WeaponType == EWeaponType::EWT_Flag && XCharacter->GetTeam() == TeamType) return;
		if (XCharacter->IsHoldingTheFlag()) return;
		XCharacter->SetOverlappingWeapon(this);
	}
}

void AWeaponParent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && XCharacter->GetTeam() == TeamType) return;
		if (XCharacter->IsHoldingTheFlag()) return;
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
	//当装备上时应当关闭UI和球体碰撞
	ShowPickUpWidget(false);
	//禁用球体碰撞
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//如果装备的使SMG将开启模拟物理
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	//绑定HighPing的委托
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter && bUseServerSideRewide)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController && HasAuthority() && !XBlasterPlayerController->HighPingDelegate.IsBound())
		{
			XBlasterPlayerController->HighPingDelegate.AddDynamic(this, &AWeaponParent::OnPingTooHigh);
		}
	}
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
	//重置回默认状态
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	//取消HighPing的委托
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter && bUseServerSideRewide)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController && HasAuthority() && XBlasterPlayerController->HighPingDelegate.IsBound())
		{
			XBlasterPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeaponParent::OnPingTooHigh);
		}
	}
}

void AWeaponParent::HandleOnSecond()
{
	//当装备上时应当关闭UI和球体碰撞
	ShowPickUpWidget(false);
	//禁用球体碰撞
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//如果装备的使SMG将开启模拟物理
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	//EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();

	//取消HighPing的委托
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter && bUseServerSideRewide)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController && HasAuthority() && XBlasterPlayerController->HighPingDelegate.IsBound())
		{
			XBlasterPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeaponParent::OnPingTooHigh);
		}
	}
}

//用于绑BlasterController中的委托
void AWeaponParent::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewide = !bPingTooHigh;
}

