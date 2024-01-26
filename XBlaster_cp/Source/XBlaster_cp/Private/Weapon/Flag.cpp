// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

AFlag::AFlag()
{
	FlagSMComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagSMComp"));
	SetRootComponent(FlagSMComp);
	SphereComp->SetupAttachment(FlagSMComp);
	PickUpWidgetComp->SetupAttachment(FlagSMComp);
}
