// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpActorParent.generated.h"

UCLASS()
class XBLASTER_CP_API APickUpActorParent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUpActorParent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//����Destoyed�����������临����
	virtual void Destroyed() override;

protected:
	//�����ص��¼��ĺ�����������������
	UFUNCTION()
		virtual void OnShpereOverlap(
			UPrimitiveComponent* OverlappedComponent, 
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComponent, 
			int32 OtherBodyIndex, 
			bool bFromSweep, 
			const FHitResult& SweepResult
		);

	UPROPERTY(EditAnywhere)
		float BaseTureRate = 45.f;

private:
	UPROPERTY(EditAnywhere)
		class USphereComponent* OverlapShpere;

	UPROPERTY(EditAnywhere)
		class USoundCue* PickUpSound;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* PickUpMesh;

	/*����Buffģ��*/
	//����ʱ����
	UPROPERTY(VisibleAnywhere)
		class UNiagaraComponent* PickupEffectComponent;
	//������Ч
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* PickupEffect;

	//�ӳٰ��ص�ʱ�䣬�������ǾͿ���վ�����ɵ��ϣ�Ҳ��������Pickup��
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

};
