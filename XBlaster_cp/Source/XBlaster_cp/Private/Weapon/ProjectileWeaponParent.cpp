// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileActor.h"
#include <Kismet/KismetMathLibrary.h>

void AProjectileWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//���������ƿ���Ĵ������Ӧ��ͨ��������ĸ��ƽ�Ч�����ݸ��ͻ���
	if (!HasAuthority())
	{
		return;
	}


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//�����ӵ�MuzzleFlash
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		//���䷽���ǹ�ڵ���Ļ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//FRotator TargetRotation = ToTarget.Rotation();
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(SocketTransform.GetLocation(), HitTarget);


		if (ProjectileClass && InstigatorPawn)
		{
			//FActorSpawnParameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectileActor>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
			}
		}
	}

}
