// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileActor.h"
#include <Kismet/KismetMathLibrary.h>

void AProjectileWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	//服务器控制开火的处理和响应，通过设置类的复制将效果传递给客户端
	if (!HasAuthority())
	{
		return;
	}


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//生成子弹MuzzleFlash
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		//发射方向从枪口到屏幕中心
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
