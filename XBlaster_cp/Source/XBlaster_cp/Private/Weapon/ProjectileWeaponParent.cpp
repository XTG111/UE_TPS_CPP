// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileActor.h"
#include <Kismet/KismetMathLibrary.h>

void AProjectileWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//�����ӵ�MuzzleFlash
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		//���䷽���ǹ�ڵ���Ļ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		//FRotator TargetRotation = ToTarget.Rotation();
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(SocketTransform.GetLocation(), HitTarget);
		//FActorSpawnParameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		
		AProjectileActor* SpawnedProjectile = nullptr;
		if (bUseServerSideRewide)
		{
			//Server
			if (InstigatorPawn->HasAuthority())
			{
				//Server,host - ���ɿɸ��Ƶ��ӵ��Ҳ�����SSR���ӵ�
				if (InstigatorPawn->IsLocallyControlled())
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					//���������˺������ɸ��Ƶ��ӵ����������ܽ��˺�������������
					SpawnedProjectile->DamageBaseFloat = Damage;
				}
				//Server not LocallyControlled - ���ɲ��ɸ����Ҿ���SSR���ӵ�
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			//Client ʹ��SSR
			else
			{
				//Client host -ʹ�ò��ɸ����Ҿ���SSR���ӵ�
				if (InstigatorPawn->IsLocallyControlled())
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeedForBullet;
					//ͬ�������˺�Ҳ��Ҫ�����ڱ��ؿͻ�����
					SpawnedProjectile->DamageBaseFloat = Damage;
				}
				//Client not LocallyControled - ���ɲ��ɸ����Ҳ�����SSR���ӵ�
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		//�������û��ʹ��SSR--ֻ�ڷ������������ɸ��Ƶ�û��SSR���ӵ�
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectileActor>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				//���������˺������ɸ��Ƶ��ӵ����������ܽ��˺�������������
				SpawnedProjectile->DamageBaseFloat = Damage;
			}
		}

	}
}
