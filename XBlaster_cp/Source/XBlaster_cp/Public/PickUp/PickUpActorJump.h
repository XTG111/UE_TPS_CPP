// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUpActorParent.h"
#include "PickUpActorJump.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API APickUpActorJump : public APickUpActorParent
{
	GENERATED_BODY()
public:
	APickUpActorJump();

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
		float JumpZHight = 2200.f;

	UPROPERTY(EditAnywhere)
		float JumpBuffTime = 10.f;
	
};
