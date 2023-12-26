// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileActor.h"

void AProjectileWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//生成子弹MuzzleFlash
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		//发射方向从枪口到屏幕中心
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();


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
