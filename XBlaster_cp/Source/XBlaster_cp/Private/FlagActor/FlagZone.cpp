// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagActor/FlagZone.h"
#include "Weapon/Flag.h"
#include "Components/SphereComponent.h"
#include "GameMode/CaptureTheFlagGameMode.h"


// Sets default values
AFlagZone::AFlagZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
}

// Called when the game starts or when spawned
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereBeginOverlap);
	
}

void AFlagZone::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//角色持有的Flag
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->TeamType != TeamType)
	{
		ACaptureTheFlagGameMode* CPGameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (CPGameMode)
		{
			CPGameMode->FlagCaptured(OverlappingFlag, this);
		}
		//在客户端进行
		OverlappingFlag->ResetFlag();
	}
}

