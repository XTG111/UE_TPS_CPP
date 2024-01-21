// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "BlasterComponent/LagCompensationComponent.h"
#include "PlayerController/XBlasterPlayerController.h"


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
		AXCharacter* HitCharacter = Cast<AXCharacter>(FireHit.GetActor());
		if (HitCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewide || OwnerPawn->IsLocallyControlled();
			//如果是服务器上的权威Actor开枪
			if (HasAuthority() && bCauseAuthDamage)
			{
				//伤害判定在服务器上进行
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			//如果只是客户端上的Actor开枪，且开启了ServeRewide
			if(!HasAuthority() && bUseServerSideRewide)
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

//整合计算射线检测目标的功能
void AHitScanWeaponParent::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	//是否开启散布，开启了，就调用上面那个随机计算结果的函数，不开启就是终点就是准星位置
	//FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
	
	//我们将在本地计算散布所以不需要在服务器中判断是否开启了
	FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
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

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
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
