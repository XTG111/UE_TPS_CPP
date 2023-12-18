// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XBLASTER_CP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//友元类，访问私有变量
	friend class AXCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	//角色实例
	UPROPERTY(VisibleAnywhere)
	class AXCharacter* CharacterEx;
	//武器实例
	UPROPERTY(VisibleAnywhere)
	class AWeaponParent* EquippedWeapon;

public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);

		
};
