// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickUpActoSpeed.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "Character/XCharacter.h"

APickUpActoSpeed::APickUpActoSpeed()
{
	bReplicates = true;
}

void APickUpActoSpeed::OnShpereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnShpereOverlap(OverlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);
	AXCharacter* XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		UXPropertyComponent* PropertyComponent = XCharacter->GetPropertyComp();
		if (PropertyComponent)
		{
			PropertyComponent->SpeedBuff(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	Destroy();
}
