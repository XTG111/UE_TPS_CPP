// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUpActorParent.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "PickUpActorAmmo.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API APickUpActorAmmo : public APickUpActorParent
{
	GENERATED_BODY()

protected:
	virtual void OnShpereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(EditAnywhere)
		int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;
};
