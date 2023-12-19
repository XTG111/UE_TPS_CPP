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

	//ͨ��������ͻ��˴��ݷ������Ĵ����� EquippedWeapon
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//��Ԫ�࣬����˽�б���
	friend class AXCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//���������������Ƿ�����׼������RPC��ʹ��
	UFUNCTION()
		void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	//�ͻ����ϳ�ǹ��ʱ��ı������ת��ʽ
	UFUNCTION()
		void OnRep_EquippedWeapon();

private:
	//��ɫʵ��
	UPROPERTY(VisibleAnywhere)
	class AXCharacter* CharacterEx;


	//װ���ϵ�����ʵ��
	//EquippedWeapon�����ж��Ƿ�װ���������Ӷ��л���������Ҫ��ֵ���ͻ���
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon,VisibleAnywhere)
	class AWeaponParent* EquippedWeapon;

	//��׼״̬
	UPROPERTY(Replicated, VisibleAnywhere)
		bool bUnderAiming;

	UPROPERTY(VisibleAnywhere)
		float BaseWalkSpeed;
	UPROPERTY(VisibleAnywhere)
		float AimWalkSpeed;

public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);

		
};
