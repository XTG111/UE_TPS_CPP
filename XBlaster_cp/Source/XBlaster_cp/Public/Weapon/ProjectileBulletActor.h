// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileActor.h"
#include "ProjectileBulletActor.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AProjectileBulletActor : public AProjectileActor
{
	GENERATED_BODY()
public:
	//实际的移动组件
		AProjectileBulletActor();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit) override;
	
};
