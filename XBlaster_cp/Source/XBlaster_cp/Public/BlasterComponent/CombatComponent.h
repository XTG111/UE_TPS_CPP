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

	//����֪ͨ ������������������
	UFUNCTION(BlueprintCallable)
		void FinishSwap();

	UFUNCTION(BlueprintCallable)
		void FinishSwapSecondWeapon();

	//��������
	void IsFired(bool bPressed);

	//����֪ͨ����ͳ������ǹ��ʱ���ӵ���
	UFUNCTION(BlueprintCallable)
		void ShotGunShellReload();
	UFUNCTION(BlueprintCallable)
		void ShotGunNoFire();

	//����֪ͨ���øú�����ִ��Ͷ��������״̬���л�
	UFUNCTION(BlueprintCallable)
		void ThrowGrenadeFinished();
	//�ӳ�����
	UFUNCTION(BlueprintCallable)
		void LaunchGrenade();
	//ServerRPC��Ͷ��Ŀ��㴫��������
	UFUNCTION(Server, Reliable)
		void ServerLauncherGrenade(const FVector_NetQuantize& Target);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//���������������Ƿ�����׼������RPC��ʹ��
	UFUNCTION()
		void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	//�ͻ����ϳ�ǹ��ʱ��ı������ת��ʽ
	//����������
	UFUNCTION()
		void OnRep_EquippedWeapon();

	//���Ƹ�����
	UFUNCTION()
		void OnRep_SecondWeapon();

	//�ж��ܷ񹥻�
	void ControlFire(bool bPressed);
	//�Բ�ͬ����״̬��������д���ԵĿ�����
	void FireProjectileWeapon(bool bPressed);
	void FireHitScanWeapon(bool bPressed);
	void FireShotGunWeapon(bool bPressed);

	//RPC��������ǹ������Ŀ�굽������
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerShotGunFire(bool bPressed, const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastShotGunFire(bool bPressed, const TArray<FVector_NetQuantize>& TraceHitTargets);

	//RPC�������״̬�������Ƿ�ǹ���Ϳͻ���׼��λ��Call from Client Do in the server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget, float FireDelay);
	//Multicast RPC Call from Server
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//���ز��ſ���Ķ����������ӳ�Ӱ��
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	//����ǹ�ı��ؿ�����ƣ�����ͳ�����е�
	void LocalShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

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

	//���׵�UI
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 GrenadeAmount = 4;
	UFUNCTION()
		void OnRep_Grenades();
	UPROPERTY(EditAnywhere)
		int32 MaxGrenade = 4;

	void UpdateHUDGrenade();

	//����
	UFUNCTION(Server, Reliable)
		void ServerDropWeapon();

	//Ͷ��
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
		void ServerThrowGrenade();

	//Ͷ�����׵���
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectileActor> GrenadeClass;

private:
	//��ɫʵ��
	UPROPERTY(VisibleAnywhere)
		AXCharacter* CharacterEx;

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

	//������
	UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon, VisibleAnywhere)
		AWeaponParent* SecondWeapon;

	//��׼״̬
	UPROPERTY(ReplicatedUsing = OnRep_UnderAming, VisibleAnywhere)
		bool bUnderAiming;
	//���ڱ����ж����������Ƿ�����׼
	bool bLocalAiming = false;

	UFUNCTION()
		void OnRep_UnderAming();

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
	//��󱸵���
	UPROPERTY(EditAnywhere)
		int32 MaxAmmoAmount = 200;
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
	//��ǹ������ʼֵ
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 GrenadeLauncherAmmo = 10;
	//��ʼ��Hash
	void InitializeCarriedAmmo();	

	//ս��״̬�Ƿ񻻵�
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();

	/*Ĭ������*/
	UPROPERTY(EditAnywhere)
		TSubclassOf<AWeaponParent> DefaultWeaponClass;


	//����װ��������һЩ������д
protected:
	void ChangeEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackPackage(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeaponParent* WeaponToEquip);
	void ReloadWeaponAutomatic();
	//����ǹе
	void DropEquippedWeapon();
	//Ͷ��ʱ��ʾ����
	void ShowAttachedGrenade(bool bShowGrenade);
	//��������
	void SwapWeapon();

public:	
	void EquipWeapon(AWeaponParent* WeaponToEquip);
	//װ��������
	void EquipPrimaryWeapon(AWeaponParent* WeaponToEquip);
	//װ��������
	void EquipSecondWeapon(AWeaponParent* WeaponToEquip);
	void NewEquipWeapon();
	//�Ƿ����㹻���ӵ�
	bool HaveAmmoCanFire();

	//����װ���Ĳ��ſ���
	void ReloadWeapon();

	//�����ӵ��ͱ�����
	void UpdateAmmoAndCarriedAmmo();

	//ר��������ǹ
	void ShotGunUpdateAmmoAndCarriedAmmo();

	//������ת��section�������ͻ��˺ͷ����������Ե���
	void JumpToShotGunEnd();

	//����PickUp����ʰȡ��ı�����
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	//��ǹʱ��ǹ�ڵ��ӵ���
	UPROPERTY(VisibleAnywhere)
		int32 SavedAmmo;

	//����Ĭ������
	void SpawnDefaultWeapon();
	//װ��Ĭ����������¶�Ӧ��UI
	void UpdateHUDAmmo();

	//��ȡ����
	FORCEINLINE AWeaponParent* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE bool GetbAiming() const { return bUnderAiming; }
	FORCEINLINE int32 GetGrenades() const { return GrenadeAmount; }
	//�ܷ񽻻�����
	bool CouldSwapWeapon();
	//���ؿ����Ƿ���FBRIK����Ϊ�����ڱ�����ǰ�����˻�������
	UPROPERTY(VisibleAnywhere)
		bool bLocallyReloading = false;

};
