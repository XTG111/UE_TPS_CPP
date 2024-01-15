// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPointParent.generated.h"

UCLASS()
class XBLASTER_CP_API APickUpSpawnPointParent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUpSpawnPointParent();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//用于设置要生成哪一个PickUp类
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<class APickUpActorParent>> PickUpClass;
	UPROPERTY(EditAnywhere)
		APickUpActorParent* SpawnedPickUp;

	//生成函数
	void SpawnPickUp();

	//SpawnPickUp定时器
	void SpawnPickUpTimerFinished();

	//设为OnDestroyed委托的回调函数，当被广播时调用
	UFUNCTION()
		void StartSpawnPickUpTimer(AActor* DestroyedActor);

private:
	//控制PickUp类的生成时间
	FTimerHandle SpawnPickUpTimer;

	//设置生成延迟的时间范围
	UPROPERTY(EditAnywhere)
		float SpawnTimeMax;
	UPROPERTY(EditAnywhere)
		float SpawnTimeMin;
};
