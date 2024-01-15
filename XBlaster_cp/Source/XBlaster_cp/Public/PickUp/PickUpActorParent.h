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

	//重载Destoyed函数，利用其复制性
	virtual void Destroyed() override;

protected:
	//被绑定重叠事件的函数，在子类中重载
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

	/*用于Buff模块*/
	//销毁时生成
	UPROPERTY(VisibleAnywhere)
		class UNiagaraComponent* PickupEffectComponent;
	//粒子特效
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* PickupEffect;

	//延迟绑定重叠时间，这样我们就可以站在生成点上，也可以生成Pickup，
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

};
