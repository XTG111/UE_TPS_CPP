// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUpActorParent.h"
#include "PickUpActorHealth.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API APickUpActorHealth : public APickUpActorParent
{
	GENERATED_BODY()
public:
	APickUpActorHealth();
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
	//治愈量
	UPROPERTY(EditAnywhere)
		float HealthAmount = 50.f;
	//治疗时间
	UPROPERTY(EditAnywhere)
		float HealingTime = 5.f;

};
