// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileActor.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AProjectileGrenade : public AProjectileActor
{
	GENERATED_BODY()
public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	//FOnProjectileBounceDelegate, ¶¯Ì¬¶à²¥£¬ ProjectileµÄµ¯Ìø
	UFUNCTION()
		void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
		class USoundCue* BounceCue;
};
