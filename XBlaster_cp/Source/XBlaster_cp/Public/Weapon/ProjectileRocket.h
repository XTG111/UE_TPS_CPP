// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileActor.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AProjectileRocket : public AProjectileActor
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	
protected:
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RocketMeshComp;
};
