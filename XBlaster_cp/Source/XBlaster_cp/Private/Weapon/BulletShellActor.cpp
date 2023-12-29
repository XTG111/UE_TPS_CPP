// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletShellActor.h"

// Sets default values
ABulletShellActor::ABulletShellActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BulletShellComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellComp"));
	SetRootComponent(BulletShellComp);

}

// Called when the game starts or when spawned
void ABulletShellActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABulletShellActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

