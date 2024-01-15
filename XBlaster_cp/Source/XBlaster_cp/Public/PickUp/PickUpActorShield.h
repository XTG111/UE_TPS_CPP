// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp/PickUpActorParent.h"
#include "PickUpActorShield.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API APickUpActorShield : public APickUpActorParent
{
	GENERATED_BODY()
public:
	APickUpActorShield();
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
	//�ָ�������
	UPROPERTY(EditAnywhere)
		float ShieldAmount = 50.f;
	//�ָ�����ʱ��
	UPROPERTY(EditAnywhere)
		float ShieldTime = 5.f;
	
};
