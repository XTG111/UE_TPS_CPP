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

	//��������Ҫ������һ��PickUp��
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<class APickUpActorParent>> PickUpClass;
	UPROPERTY(EditAnywhere)
		APickUpActorParent* SpawnedPickUp;

	//���ɺ���
	void SpawnPickUp();

	//SpawnPickUp��ʱ��
	void SpawnPickUpTimerFinished();

	//��ΪOnDestroyedί�еĻص������������㲥ʱ����
	UFUNCTION()
		void StartSpawnPickUpTimer(AActor* DestroyedActor);

private:
	//����PickUp�������ʱ��
	FTimerHandle SpawnPickUpTimer;

	//���������ӳٵ�ʱ�䷶Χ
	UPROPERTY(EditAnywhere)
		float SpawnTimeMax;
	UPROPERTY(EditAnywhere)
		float SpawnTimeMin;
};
