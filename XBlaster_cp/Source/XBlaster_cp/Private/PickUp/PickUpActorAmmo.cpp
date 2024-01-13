// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickUpActorAmmo.h"
#include "BlasterComponent/CombatComponent.h"
#include "Character/XCharacter.h"

void APickUpActorAmmo::OnShpereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnShpereOverlap(OverlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, SweepResult);
	AXCharacter* XCharacter = Cast<AXCharacter>(OtherActor);
	if (XCharacter)
	{
		UCombatComponent* CombatComp = XCharacter->GetCombatComp();
		if (CombatComp)
		{
			CombatComp->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
