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
	//������
	UPROPERTY(EditAnywhere)
		float HealthAmount = 50.f;
	//����ʱ��
	UPROPERTY(EditAnywhere)
		float HealingTime = 5.f;

};
