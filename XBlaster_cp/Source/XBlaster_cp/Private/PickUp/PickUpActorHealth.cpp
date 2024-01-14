// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickUpActorHealth.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "Character/XCharacter.h"


APickUpActorHealth::APickUpActorHealth()
{
	bReplicates = true;
}

void APickUpActorHealth::OnShpereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnShpereOverlap(OverlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);
	AXCharacter* XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		UXPropertyComponent* PropertyComponent = XCharacter->GetPropertyComp();
		if (PropertyComponent)
		{
			PropertyComponent->HealCharacter(HealthAmount, HealingTime);
		}
	}
	Destroy();
}
