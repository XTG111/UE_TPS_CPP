// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/SnipperWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterComponent/CombatComponent.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "BlasterComponent/LagCompensationComponent.h"

void ASnipperWeaponParent::Fire(const FVector& HitTarget)
{
	//直接调用武器父类中的开火函数
	AWeaponParent::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	AXCharacter* OwnerCharacter = Cast<AXCharacter>(OwnerPawn);
	bUnderAiming = OwnerCharacter->GetCombatComp()->GetbAiming();
	bUseScatter = !bUnderAiming;

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

		SnipperTraceHit(Start, HitTarget, FireHit);

		//伤害计算
		AXCharacter* HitCharacter = Cast<AXCharacter>(FireHit.GetActor());
		if (HitCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewide || OwnerPawn->IsLocallyControlled();
			//如果是服务器上的权威Actor开枪
			if (HasAuthority() && bCauseAuthDamage)
			{
				//是否击中头部
				bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				const float DamageToCause = bHeadShot ? HeadShotDamage : Damage;
				//伤害判定在服务器上进行
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			//如果只是客户端上的Actor开枪，且开启了ServeRewide
			else if (!HasAuthority() && bUseServerSideRewide)
			{
				XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
				XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(InstigatorController) : XBlasterPlayerController;
				if (XCharacter && XBlasterPlayerController && XCharacter->GetLagCompensationComp() && XCharacter->IsLocallyControlled())
				{
					XCharacter->GetLagCompensationComp()->ServerScoreRequest(
						HitCharacter,
						Start,
						HitTarget,
						XBlasterPlayerController->GetSeverTime() - XBlasterPlayerController->SingleTripTime,
						this
					);
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

}

void ASnipperWeaponParent::SnipperTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	//是否开启散布，开启了，就调用上面那个随机计算结果的函数，不开启就是终点就是准星位置
	FVector End =  bUseScatter ? TraceEndWithScatter(HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
	//FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
	
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
		else
		{
			OutHit.ImpactPoint = End;
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
