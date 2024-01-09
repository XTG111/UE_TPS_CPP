// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

//��ս������л����öಥRPC����Fire
void AHitScanWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//Controllerֻ�����ڱ��أ�������ģ��Actor�Ļ����϶��ǲ����ڵģ�
	//�����������������Χ�ж�ʹ����&& InstigatorController ��ô���������������Ϲ۲ⲻ��������Чֻ�����˺���Ӧ
	if (MuzzleFlashSocket)
	{
		FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
		//��ǹ�ڳ���
		FVector Start = SockertTransform.GetLocation();
		//�յ�λ����׼�����߼���յ� *1.25Ϊ��ȷ��һ����⵽
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
			//�������
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				XCharacter = Cast<AXCharacter>(FireHit.GetActor());
				if (XCharacter && HasAuthority() && InstigatorController)
				{
					//����������ÿ���ͻ��˶��������߼������������˺��ж��ڷ������Ͻ���
					UGameplayStatics::ApplyDamage(
						XCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);

				}
				//���к����������Ч
				if (ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticle,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				//���ǹ�Ļ�����Ч
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
