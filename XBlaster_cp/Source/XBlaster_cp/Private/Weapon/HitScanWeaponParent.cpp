// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

//在战斗组件中会利用多播RPC调用Fire
void AHitScanWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	//获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//Controller只存在于本地，在其他模拟Actor的机器上都是不存在的，
	//所以如果在这里最外围判断使用了&& InstigatorController 那么将导致其他机器上观测不到粒子特效只会有伤害反应
	if (MuzzleFlashSocket)
	{
		FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
		//从枪口出发
		FVector Start = SockertTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		//伤害计算
		XCharacter = Cast<AXCharacter>(FireHit.GetActor());
		if (XCharacter && HasAuthority() && InstigatorController)
		{
			//由于我们是每个客户端都进行射线检测射击，所以伤害判定在服务器上进行
			UGameplayStatics::ApplyDamage(
				XCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);

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
				FireHit.ImpactPoint
			);
		}
		SetSubMachineGunProper(GetWorld(), SockertTransform);
	}
}

void AHitScanWeaponParent::SetSubMachineGunProper(UWorld* World, FTransform& SockertTransform)
{
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			World,
			MuzzleFlash,
			SockertTransform
		);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			GetActorLocation()
		);
	}
}

FVector AHitScanWeaponParent::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	//从起点指向目标的方向向量
	FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	
	//散布圆心位置
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);

	//随机生成球内1点
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	
	FVector ToEndLoc = EndLoc - TraceStart;
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * 80000.f / ToEndLoc.Size()), FColor::Cyan, true);
	return FVector(TraceStart + ToEndLoc * 80000.f/ToEndLoc.Size());
}

//整合计算射线检测目标的功能
void AHitScanWeaponParent::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	//是否开启散布，开启了，就调用上面那个随机计算结果的函数，不开启就是终点就是准星位置
	FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
	if (World)
	{
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (BeamComp)
			{
				BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
