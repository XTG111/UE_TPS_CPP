// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/LagCompensationComponent.h"
#include "Character/XCharacter.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponParent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	SaveFramePackage_Tick();
}

void ULagCompensationComponent::SaveFramePackage_Tick()
{
	if (XCharacter == nullptr || !XCharacter->HasAuthority()) return;
	//下列获取FramePackage只在服务器上进行
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
	if (XCharacter)
	{
		Package.Character = XCharacter;
		//因为只在服务器上所以直接获取当前世界时间
		Package.Time = GetWorld()->GetTimeSeconds();
		//存储每个Box的信息
		for (auto& HitBoxPair : XCharacter->HitBoxCompMap)
		{
			FBoxInfomation BoxInformation;
			BoxInformation.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			//将信息存到FramePackage的Map中
			Package.HitBoxInfoMap.Add(HitBoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxInfoMap)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	//服务器用来检测是否击中的那一个Frame,存储最终结果
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	//检测是否击中
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunServerSideRewind(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for (AXCharacter* HitCharacter : HitCharacters)
	{
		//获取霰弹枪的击中检测Box信息
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotGunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}
FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(AXCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();

	//服务器用来检测是否击中的那一个Frame,存储最终结果
	FFramePackage FrameToCheck;
	bool bShouldInterp = true;

	//存储被击中角色的FrameHistory
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	//记录的最早时间
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	//记录的最晚时间
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime)
	{
		//击中的时间太久远，不应该来判断
		return  FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		bShouldInterp = false;
		FrameToCheck = History.GetTail()->GetValue();
	}
	if (NewestHistoryTime <= HitTime)
	{
		bShouldInterp = false;
		//这种情况不应该存在
		FrameToCheck = History.GetHead()->GetValue();
	}

	//HitTime between Oldest and Newest
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* YoungerNode = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* OlderNode = History.GetHead();
	//OlderTime < HitTime < YoungerTime
	while (OlderNode->GetValue().Time > HitTime)
	{
		if (OlderNode->GetNextNode() == nullptr) break;
		OlderNode = OlderNode->GetNextNode();
		if (OlderNode->GetValue().Time > HitTime)
		{
			YoungerNode = OlderNode;
		}
	}
	if (OlderNode->GetValue().Time == HitTime)
	{
		bShouldInterp = false;
		FrameToCheck = OlderNode->GetValue();
	}
	//插值计算准确值
	if (bShouldInterp)
	{
		//Interp between YougerNode and OlderNode
		FrameToCheck = InterpBetweenFrames(OlderNode->GetValue(), YoungerNode->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	//检测是否击中
	return FrameToCheck;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	//计算起点和终点的长度
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	//求得待求点相对于起点的偏移量
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	//更新InterpFP中的其他信息
	for (auto& YoungerPair : YoungerFrame.HitBoxInfoMap)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		//获取OlderTime的Box信息
		const FBoxInfomation& OlderBox = OlderFrame.HitBoxInfoMap[BoxInfoName];
		//获取YoungerTime的Box信息
		const FBoxInfomation& YoungerBox = YoungerPair.Value;

		//插值
		FBoxInfomation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = OlderBox.BoxExtent;

		InterpFramePackage.HitBoxInfoMap.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

void ULagCompensationComponent::CacheBoxPosition(AXCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCompMap)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInfomation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfoMap.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(AXCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCompMap)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfoMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfoMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfoMap[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetBoxes(AXCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitBoxCompMap)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfoMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfoMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfoMap[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(AXCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnable);
	}
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	UE_LOG(LogTemp, Warning, TEXT("FireHit"));
	//回退之前的Box参数
	FFramePackage CurrentFrame;
	//存储当前Box信息到CurrentFrame中
	CacheBoxPosition(HitCharacter, CurrentFrame);
	//Package为要回退到的位置
	MoveBoxes(HitCharacter, Package);

	//关闭被击中角色的骨骼网格体碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//先启用头部BOX的碰撞 来检测是否击中
	UBoxComponent* HeadBox = HitCharacter->HitBoxCompMap[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraveEnd = TraceStart + (HitLocation - TraceStart) * 5.f;
	UWorld* World = GetWorld();

	if (World)
	{
		World->LineTraceSingleByObjectType(
			ConfirmHitResult,
			TraceStart,
			TraveEnd,
			ECC_HitBox
		);
		//如果击中了头部
		if (ConfirmHitResult.bBlockingHit)
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent() * 10.f, FQuat(Box->GetComponentRotation()), FColor::Red, false, 5.f);
				}
			}
			ResetBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true,true };
		}
		//如果没有击中头部,检测其他Box
		else
		{
			//为其他Box启用碰撞
			for (auto& HitBoxPair : HitCharacter->HitBoxCompMap)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			class TArray<AActor*> IgnoreA;
			TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_HitBox));
			//开始检测
			bool bHitReal = UKismetSystemLibrary::SphereTraceSingleForObjects(
				World,
				TraceStart,
				TraveEnd,
				5.f,
				ObjectTypes,
				false,
				IgnoreA,
				EDrawDebugTrace::None,
				ConfirmHitResult,
				true,
				FColor::Red,
				FColor::Cyan,
				5.f
			);
			if (bHitReal)
			{
				UE_LOG(LogTemp, Warning, TEXT("Body"));
				if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent() * 10.f, FQuat(Box->GetComponentRotation()), FColor::Cyan, false, 5.f);
					}
				}

				ResetBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true,false };
			}
		}
	}
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false,false };
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr)
		{
			return FShotGunServerSideRewindResult();
		}
	}

	FShotGunServerSideRewindResult ShotGunResult;
	//同样保存当前帧的Box
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		//存储当前Box信息到CurrentFrame中
		CacheBoxPosition(Frame.Character, CurrentFrame);
		//Frame为要回退到的位置
		MoveBoxes(Frame.Character, Frame);
		//关闭被击中角色的骨骼网格体碰撞
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		//先启用头部BOX的碰撞 来检测是否击中
		UBoxComponent* HeadBox = Frame.Character->HitBoxCompMap[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}
	
	UWorld* World = GetWorld();

	//射线检测是否击中了头部
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraveEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		if (World)
		{
			World->LineTraceSingleByObjectType(
				ConfirmHitResult,
				TraceStart,
				TraveEnd,
				ECC_HitBox
			);
			AXCharacter* HitCharacter = Cast<AXCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (ShotGunResult.HeadShots.Contains(HitCharacter))
				{
					ShotGunResult.HeadShots[HitCharacter]++;
				}
				else
				{
					ShotGunResult.HeadShots.Emplace(HitCharacter, 1);
				}
			}
		}
	}
	//开启其他骨骼的Box碰撞
	for (auto& Frame : FramePackages)
	{
		//为其他Box启用碰撞
		for (auto& HitBoxPair : Frame.Character->HitBoxCompMap)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		//关闭头部的碰撞
		UBoxComponent* HeadBox = Frame.Character->HitBoxCompMap[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//射线检测是否击中了身体
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraveEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

		if (World)
		{
			class TArray<AActor*> IgnoreA;
			TArray<TEnumAsByte<EObjectTypeQuery> > ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_HitBox));
			//开始检测
			bool bHitReal = UKismetSystemLibrary::SphereTraceSingleForObjects(
				World,
				TraceStart,
				TraveEnd,
				5.f,
				ObjectTypes,
				false,
				IgnoreA,
				EDrawDebugTrace::None,
				ConfirmHitResult,
				true,
				FColor::Red,
				FColor::Cyan,
				5.f
			);
			//World->LineTraceSingleByObjectType(
			//	ConfirmHitResult,
			//	TraceStart,
			//	TraveEnd,
			//	ECC_HitBox
			//);
			AXCharacter* HitCharacter = Cast<AXCharacter>(ConfirmHitResult.GetActor());
			if (HitCharacter)
			{
				if (ShotGunResult.BodyShots.Contains(HitCharacter))
				{
					ShotGunResult.BodyShots[HitCharacter]++;
				}
				else
				{
					ShotGunResult.BodyShots.Emplace(HitCharacter, 1);
				}
			}
		}
	}
	//恢复之前的Box
	for (auto& Frame : CurrentFrames)
	{
		ResetBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotGunResult;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	//回退之前的Box参数
	FFramePackage CurrentFrame;
	//存储当前Box信息到CurrentFrame中
	CacheBoxPosition(HitCharacter, CurrentFrame);
	//Package为要回退到的位置
	MoveBoxes(HitCharacter, Package);

	//关闭被击中角色的骨骼网格体碰撞
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//先启用头部BOX的碰撞 来检测是否击中
	UBoxComponent* HeadBox = HitCharacter->HitBoxCompMap[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	/*预测轨迹*/
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	/*Debug*/
	/*PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;*/
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	//当击中头部返回
	if (PathResult.HitResult.bBlockingHit)
	{
		//如果击中了头部
		//if (PathResult.HitResult.Component.IsValid())
		//{
		//	UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
		//	if (Box)
		//	{
		//		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 5.f);
		//	}
		//}
		ResetBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true,true };
	}
	//没有击中头部，检测是否击中其他部位
	else
	{
		//为其他Box启用碰撞
		for (auto& HitBoxPair : HitCharacter->HitBoxCompMap)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{

			ResetBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true,false };
		}
	}
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false,false };
}

//ServerRPC_Implementation
void ULagCompensationComponent::ServerScoreRequest_Implementation(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeaponParent* DamageCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (XCharacter && HitCharacter && DamageCauser && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? DamageCauser->HeadShotDamage : DamageCauser->Damage;
		//应用伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			XCharacter->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

//霰弹枪Server
void ULagCompensationComponent::ServerShotGunScoreRequest_Implementation(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotGunServerSideRewindResult Confirm = ShotGunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr ||XCharacter == nullptr || XCharacter->GetEquippedWeapon() == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * XCharacter->GetEquippedWeapon()->HeadShotDamage;
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * XCharacter->GetEquippedWeapon()->Damage;
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			XCharacter->Controller,
			XCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

//Projectile Server
void ULagCompensationComponent::ServerProjectileScoreRequest_Implementation(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	if (XCharacter && HitCharacter && Confirm.bHitConfirmed && XCharacter->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? XCharacter->GetEquippedWeapon()->HeadShotDamage : XCharacter->GetEquippedWeapon()->Damage;
		//应用伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			XCharacter->Controller,
			XCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

