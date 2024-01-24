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
//	//ֱ�ӵ������������еĿ�����
//	AWeaponParent::Fire(HitTarget);
//	APawn* OwnerPawn = Cast<APawn>(GetOwner());
//	if (OwnerPawn == nullptr) return;
//	AController* InstigatorController = OwnerPawn->GetController();
//	//��ȡǹ��λ��
//	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
//	//Controllerֻ�����ڱ��أ�������ģ��Actor�Ļ����϶��ǲ����ڵģ�
//	//�����������������Χ�ж�ʹ����&& InstigatorController ��ô���������������Ϲ۲ⲻ��������Чֻ�����˺���Ӧ
//	if (MuzzleFlashSocket)
//	{
//		FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
//		//��ǹ�ڳ���
//		FVector Start = SockertTransform.GetLocation();
//		//�洢���еĽ�ɫ��ÿ����ɫ�ܵ����ӵ���
//		TMap<AXCharacter*, uint32> HitMap;
//
//		//����10�����λ��
//		for (uint32 i = 0; i < NumberOfPellets; i++)
//		{
//			FHitResult FireHit;
//			WeaponTraceHit(Start, HitTarget, FireHit);
//
//			//�˺�����
//			XCharacter = Cast<AXCharacter>(FireHit.GetActor());
//			if (XCharacter && HasAuthority() && InstigatorController)
//			{
//				//���������ôHits������
//				if (HitMap.Contains(XCharacter))
//				{
//					HitMap[XCharacter]++;
//				}
//				else
//				{
//					HitMap.Emplace(XCharacter, 1);
//				}
//			}
//			//���߼���������к����������Ч
//			if (ImpactParticle)
//			{
//				UGameplayStatics::SpawnEmitterAtLocation(
//					GetWorld(),
//					ImpactParticle,
//					FireHit.ImpactPoint,
//					FireHit.ImpactNormal.Rotation()
//				);
//			}
//			//���߼�������Ļ�����Ч
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

	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//Controllerֻ�����ڱ��أ�������ģ��Actor�Ļ����϶��ǲ����ڵģ�
	//�����������������Χ�ж�ʹ����&& InstigatorController ��ô���������������Ϲ۲ⲻ��������Чֻ�����˺���Ӧ
	if (MuzzleFlashSocket)
	{
		const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
		//��ǹ�ڳ���
		const FVector Start = SockertTransform.GetLocation();
		//map to hit character to number of times hit
		TMap<AXCharacter*, uint32> HitMap;
		TMap<AXCharacter*, uint32> HeadShotHitMap;
		for (auto HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			//�˺�����,ֻ�ڱ��ؽ��У��������ͱ��ض���ͳ��
			AXCharacter* HitCharacter = Cast<AXCharacter>(FireHit.GetActor());
			if (HitCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				//���������ôHits������
				if (HitMap.Contains(HitCharacter))
				{
					HitMap[HitCharacter]++;
				}
				else
				{
					HitMap.Emplace(HitCharacter, 1);
				}

				//�����Ƿ����ͷ��
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
			//���߼���������к����������Ч
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticle,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			//���߼�������Ļ�����Ч
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
				//������ڷ������ϵĿ�����ôֱ�ӵ���
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

		//����ڿͻ�����
		if (!HasAuthority() && bUseServerSideRewide)
		{
			XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(OwnerPawn) : XCharacter;
			XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(InstigatorController) : XBlasterPlayerController;
			//ֻ���ڱ��ش������RPC��������Ϊ���ǵĿ����ܲ�û��д�ڷ������ϣ����пͻ��˶�����Ӧ�����˺�����ֻ��ͨ�����ص�actor
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
	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//��������Ҫ�ڱ��ؼ���ɢ������ֱ�Ӵ���·���Ŀ�ʼλ��
	if (MuzzleFlashSocket == nullptr) return ;
	const FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	//��ǹ�ڳ���
	const FVector TraceStart = SockertTransform.GetLocation();
	//�����ָ��Ŀ��ķ�������
	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();
	//ɢ��Բ��λ��
	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		//�����������1��
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
