// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"

//��ս������л����öಥRPC����Fire
void AHitScanWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//if (!HasAuthority())
	//{
	//	return;
	//}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigatorController)
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
			//�������
			if (FireHit.bBlockingHit)
			{
				XCharacter = Cast<AXCharacter>(FireHit.GetActor());
				if (XCharacter)
				{
					//����������ÿ���ͻ��˶��������߼������������˺��ж��ڷ������Ͻ���
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
							XCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
					}

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
			}
		}
	}
}
