// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/XBlasterHUD.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "XBlaster_cp/XTypeHeadFile/CombatState.h"
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

	//����װ���������޸�״̬Ϊ��CombatState
	UFUNCTION(BlueprintCallable)
		void FinishingReloading();

	//��������
	void IsFired(bool bPressed);

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

	//�ж��ܷ񹥻�
		void ControlFire(bool bPressed);

	//RPC�������״̬�������Ƿ�ǹ���Ϳͻ���׼��λ��Call from Client Do in the server
	UFUNCTION(Server, Reliable)
		void ServerFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);
	//Multicast RPC Call from Server
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	////RPC���ݻ�������
	UFUNCTION(Server, Reliable)
		void ServerReload();

	//�ڷ������Ϳͻ��������еĹ�����
	void HandleReload();

	//linetrace�����Ƶ�
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	//���׼�� ��̬׼�ǣ��漰��ֵ�����Դ������float
	void SetHUDCrossHairs(float Deltatime);

	//��������װ��
	int32 AmmountToReload();

private:
	//��ɫʵ��
	UPROPERTY(VisibleAnywhere)
		class AXCharacter* CharacterEx;

	//����һ�����������ÿ���������HUDPlayer::Controller -->APlayerController::GetHUD()
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;
	//�洢HUD
	UPROPERTY()
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
	UPROPERTY(Replicated,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

	//��ǰװ�������ϵ�����������
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
		int32 CarriedAmmo;
	UFUNCTION()
		void OnRep_CarriedAmmo();
	//�洢�����Ͷ�Ӧ�ı�����
	TMap<EWeaponType, int32> CarriedAmmoMap;
	//��ǹ��������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingARAmmo = 30;
	//���ڱ�������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingRocketAmmo = 0;
	//��ǹ��������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingPistolAmmo = 15;
	//���ǹ������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 SubMachineGunAmmo = 55;
	//����ǹ������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 ShotGunAmmo = 10;
	//�ѻ�ǹ������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 SnipperAmmo = 10;

	//��ʼ��Hash
	void InitializeCarriedAmmo();	

	//ս��״̬�Ƿ񻻵�
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();

public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);
	//�Ƿ����㹻���ӵ�
	bool HaveAmmoCanFire();

	//����װ���Ĳ��ſ���
	void ReloadWeapon();

	//�����ӵ��ͱ�����
	void UpdateAmmoAndCarriedAmmo();

	//��ȡ����
	FORCEINLINE AWeaponParent* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE bool GetbAiming() const { return bUnderAiming; }
};
