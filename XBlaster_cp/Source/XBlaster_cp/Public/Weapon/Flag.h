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
	virtual void Drop() override;
	void ResetFlag();
	
protected:
	virtual void HandleOnEquipped() override;
	virtual void HandleOnDropped() override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* FlagSMComp;

	//ÆðÊ¼Î»ÖÃ
	UPROPERTY(VisibleAnywhere)
		FTransform InitialTransform;
public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
 	
};
