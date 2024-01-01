// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/XBlasterHUD.h"
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

	//����
	void IsFired(bool bPressed);

	void ControlFire(bool bPressed);

	//RPC�������״̬�������Ƿ�ǹ���Ϳͻ���׼��λ��
	UFUNCTION(Server, Reliable)
		void ServerFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//linetrace�����Ƶ�
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	//���׼�� ��̬׼�ǣ��漰��ֵ�����Դ������float
	void SetHUDCrossHairs(float Deltatime);

private:
	//��ɫʵ��
	UPROPERTY(VisibleAnywhere)
		class AXCharacter* CharacterEx;

	//����һ�����������ÿ���������HUDPlayer::Controller -->APlayerController::GetHUD()
	class AXBlasterPlayerController* XBlasterPlayerController;
	//�洢HUD
	class AXBlasterHUD* XBlasterHUD;


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

	//����
	UPROPERTY(Replicated)
		bool bFired = false;
	//Ŀ�깥����
	FVector HitTarget;

	//HUD Crosshair
	UPROPERTY(EditAnywhere)
		FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	/*
	Aiming And FOV
	*/
	//����׼ʱ��Ĭ���ӽ�
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	//�ӽǱ任��ֵ����
	void InterpFOV(float Deltatime);

	//�Զ�����Ķ�ʱ��
	FTimerHandle FireTime;
	//��ǰ�Ŀ���ʱ���Ƿ���ɣ��������û������ͷ�����һ���ӵ�
	bool bCanFire = true;

	//��ʱ���Ŀ�ʼ�����
	void StartFireTimer();
	void FireTimeFinished();


public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);
};
