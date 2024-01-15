// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnPickUpActor/PickUpSpawnPointParent.h"
#include "PickUp/PickUpActorParent.h"

// Sets default values
APickUpSpawnPointParent::APickUpSpawnPointParent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called every frame
void APickUpSpawnPointParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the game starts or when spawned
void APickUpSpawnPointParent::BeginPlay()
{
	Super::BeginPlay();
	//初始生成
	StartSpawnPickUpTimer((AActor*)nullptr);
}

void APickUpSpawnPointParent::SpawnPickUp()
{
	int32 NumPickUpClasses = PickUpClass.Num();
	if (NumPickUpClasses > 0)
	{
		//随机选择一个类生成
		int32 Selection = FMath::RandRange(0, NumPickUpClasses - 1);
		SpawnedPickUp = GetWorld()->SpawnActor<APickUpActorParent>(PickUpClass[Selection], GetActorTransform());
		//绑定Destroyed委托
		if (HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawnPointParent::StartSpawnPickUpTimer);
		}
	}
}

void APickUpSpawnPointParent::StartSpawnPickUpTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnTimeMin, SpawnTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickUpTimer, 
		this, 
		&APickUpSpawnPointParent::SpawnPickUpTimerFinished, 
		SpawnTime
	);
}

void APickUpSpawnPointParent::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
	GetWorldTimerManager().ClearTimer(SpawnPickUpTimer);
}



