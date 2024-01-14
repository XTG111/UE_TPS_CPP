// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUpActorParent.h"
#include "PickUpActoSpeed.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API APickUpActoSpeed : public APickUpActorParent
{
	GENERATED_BODY()
public:
	APickUpActoSpeed();
		
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
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditAnywhere)
		float CrouchSpeedBuff = 850.f;
	
	UPROPERTY(EditAnywhere)
		float SpeedBuffTime = 10.f;
};
