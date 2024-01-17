// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ShotGunWeaponParent.h"
#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

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
		for (auto HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			//伤害计算,只在本地进行，服务器和本地都会统计
			XCharacter = Cast<AXCharacter>(FireHit.GetActor());
			if (XCharacter)
			{
				//如果击中那么Hits数增加
				if (HitMap.Contains(XCharacter))
				{
					HitMap[XCharacter]++;
				}
				else
				{
					HitMap.Emplace(XCharacter, 1);
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
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController && HasAuthority())
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key, //character that was hit
					Damage * HitPair.Value,//multiply damage bu number of times
					InstigatorController,
					this,
					UDamageType::StaticClass()
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
