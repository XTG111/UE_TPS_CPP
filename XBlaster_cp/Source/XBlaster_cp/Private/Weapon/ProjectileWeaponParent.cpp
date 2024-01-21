// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileActor.h"
#include <Kismet/KismetMathLibrary.h>

void AProjectileWeaponParent::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);


	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	//生成子弹MuzzleFlash
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

		//发射方向从枪口到屏幕中心
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
				//Server,host - 生成可复制的子弹且不具有SSR的子弹
				if (InstigatorPawn->IsLocallyControlled())
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					//将武器的伤害传给可复制的子弹，这样才能将伤害带到服务器上
					SpawnedProjectile->DamageBaseFloat = Damage;
				}
				//Server not LocallyControlled - 生成不可复制且具有SSR的子弹
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			//Client 使用SSR
			else
			{
				//Client host -使用不可复制且具有SSR的子弹
				if (InstigatorPawn->IsLocallyControlled())
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeedForBullet;
					//同样对于伤害也需要设置在本地客户端上
					SpawnedProjectile->DamageBaseFloat = Damage;
				}
				//Client not LocallyControled - 生成不可复制且不具有SSR的子弹
				else
				{
					SpawnedProjectile = World->SpawnActor<AProjectileActor>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		//如果武器没有使用SSR--只在服务器上生产可复制的没有SSR的子弹
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectileActor>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				//将武器的伤害传给可复制的子弹，这样才能将伤害带到服务器上
				SpawnedProjectile->DamageBaseFloat = Damage;
			}
		}

	}
}
