// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponParent.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AFlag : public AWeaponParent
{
	GENERATED_BODY()
public:
	AFlag();
private:
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* FlagSMComp;
	
};
