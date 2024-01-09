// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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
		//终点位置是准星射线检测终点 *1.25为了确保一定检测到
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd = End;
			//如果射中
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
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
				//击中后产生粒子特效
				if (ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticle,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				//冲锋枪的击中音效
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint
					);
				}
				if (BeamParticles)
				{
					UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(
						World,
						BeamParticles,
						SockertTransform
						);
					if (BeamComp)
					{
						BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
					}
				}
			}
			SetSubMachineGunProper(World, SockertTransform);
		}
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
