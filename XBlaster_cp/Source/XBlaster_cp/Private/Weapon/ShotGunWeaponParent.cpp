// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ShotGunWeaponParent.h"
#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterComponent/LagCompensationComponent.h"
#include "PlayerController/XBlasterPlayerController.h"

//void AShotGunWeaponParent::Fire(const FVector& HitTarget)
//{
//	//直接调用武器父类中的开火函数
//	AWeaponParent::Fire(HitTarget);
//	APawn* OwnerPawn = Cast<APawn>(GetOwner());
//	if (OwnerPawn == nullptr) return;
//	AController* InstigatorController = OwnerPawn->GetController();
//	//获取枪口位置
//	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
//	//Controller只存在于本地，在其他模拟Actor的机器上都是不存在的，
//	//所以如果在这里最外围判断使用了&& InstigatorController 那么将导致其他机器上观测不到粒子特效只会有伤害反应
//	if (MuzzleFlashSocket)
//	{
//		FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
//		//从枪口出发
//		FVector Start = SockertTransform.GetLocation();
//		//存储击中的角色和每个角色受到的子弹数
//		TMap<AXCharacter*, uint32> HitMap;
//
//		//生成10个随机位置
//		for (uint32 i = 0; i < NumberOfPellets; i++)
//		{
//			FHitResult FireHit;
//			WeaponTraceHit(Start, HitTarget, FireHit);
//
//			//伤害计算
//			XCharacter = Cast<AXCharacter>(FireHit.GetActor());
//			if (XCharacter && HasAuthority() && InstigatorController)
//			{
//				//如果击中那么Hits数增加
//				if (HitMap.Contains(XCharacter))
//				{
//					HitMap[XCharacter]++;
//				}
//				else
//				{
//					HitMap.Emplace(XCharacter, 1);
//				}
//			}
//			//射线检测武器击中后产生粒子特效
//			if (ImpactParticle)
//			{
//				UGameplayStatics::SpawnEmitterAtLocation(
//					GetWorld(),
//					ImpactParticle,
//					FireHit.ImpactPoint,
//					FireHit.ImpactNormal.Rotation()
//				);
//			}
//			//射线检测武器的击中音效
//			if (HitSound)
//			{
//				UGameplayStatics::PlaySoundAtLocation(
//					this,
//					HitSound,
//					FireHit.ImpactPoint,
//					0.5f,
//					FMath::FRandRange(-0.5f,0.5f)
//				);
//			}
//		}
//		for (auto HitPair : HitMap)
//		{
//			if (HitPair.Key && InstigatorController && HasAuthority())
//			{
//				UGameplayStatics::ApplyDamage(
//					HitPair.Key,
//					Damage * HitPair.Value,
//					InstigatorController,
//					this,
//					UDamageType::StaticClass()
//				);
//			}
//		}
//	}
//}

void AShotGunWeaponParent::FireShotGun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeaponParent::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	//获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//Controller只存在于本地，在其他模拟Actor的机器上都是不存在的，
	//所以如果在这里最外围判断使用了&& InstigatorController 那么将导致其他机器上观测不到粒子特效只会有伤害反应
	if (MuzzleFlashSocket)
	{
		const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
		//从枪口出发
		const FVector Start = SockertTransform.GetLocation();
		//map to hit character to number of times hit
		TMap<AXCharacter*, uint32> HitMap;
		TMap<AXCharacter*, uint32> HeadShotHitMap;
		for (auto HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			//伤害计算,只在本地进行，服务器和本地都会统计
			AXCharacter* HitCharacter = Cast<AXCharacter>(FireHit.GetActor());
			if (HitCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				//如果击中那么Hits数增加
				if (HitMap.Contains(HitCharacter))
				{
					HitMap[HitCharacter]++;
				}
				else
				{
					HitMap.Emplace(HitCharacter, 1);
				}

				//更新是否击中头部
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(HitCharacter))
					{
						HeadShotHitMap[HitCharacter]++;
					}
					else
					{
						HeadShotHitMap.Emplace(HitCharacter, 1);
					}
				}
			}
			//射线检测武器击中后产生粒子特效
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticle,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			//射线检测武器的击中音效
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					0.5f,
					FMath::FRandRange(-0.5f, 0.5f)
				);
			}
		}
		//maps character hit to total damage
		TMap<AXCharacter*, uint32> DamageMap = GetDamageMap(HitMap, HeadShotHitMap);
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				//如果是在服务器上的开火那么直接调用
				bool bCauseAuthDamage = !bUseServerSideRewide || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, //character that was hit
						DamagePair.Value,//multiply damage by number of times
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}

		//如果在客户端上
		if (!HasAuthority() && bUseServerSideRewide)
		{
			XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(OwnerPawn) : XCharacter;
			XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(InstigatorController) : XBlasterPlayerController;
			//只会在本地传递这个RPC，这是因为我们的开火功能并没有写在服务器上，所有客户端都会响应，而伤害计算只能通过本地的actor
			if (XCharacter && XBlasterPlayerController && XCharacter->GetLagCompensationComp() && XCharacter->IsLocallyControlled())
			{
				XCharacter->GetLagCompensationComp()->ServerShotGunScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					XBlasterPlayerController->GetSeverTime() - XBlasterPlayerController->SingleTripTime
				);
			}
		}
	}
}

void AShotGunWeaponParent::ShotGunTraceEndwithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	//获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//由于我们要在本地计算散布所以直接传入路径的开始位置
	if (MuzzleFlashSocket == nullptr) return ;
	const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	//从枪口出发
	const FVector TraceStart = SockertTransform.GetLocation();
	//从起点指向目标的方向向量
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	//散布圆心位置
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		//随机生成球内1点
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
		//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()), FColor::Cyan, true);
		HitTargets.Add(FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()));
	}
}

TMap<AXCharacter*, uint32> AShotGunWeaponParent::GetDamageMap(TMap<AXCharacter*, uint32>& HitMap, TMap<AXCharacter*, uint32>& HeadShotHitMap)
{
	TMap<AXCharacter*, uint32> DamageMap;
	for (auto HitPair : HitMap)
	{
		if (HitPair.Key)
		{
			DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
			HitCharacters.AddUnique(HitPair.Key);
		}
	}
	for (auto HeadShotHitPair : HeadShotHitMap)
	{
		if (HeadShotHitPair.Key)
		{
			if (DamageMap.Contains(HeadShotHitPair.Key))
			{
				DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
			}
			else
			{
				DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
			}
			HitCharacters.AddUnique(HeadShotHitPair.Key);
		}
	}

	return DamageMap;
}
