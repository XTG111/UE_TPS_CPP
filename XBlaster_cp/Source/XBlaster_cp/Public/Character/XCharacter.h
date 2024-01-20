// Fill out your copyright notice in the Description page of Project Settings.IInteractWithCrosshairInterface

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "XBlaster_cp/XTypeHeadFile/CombatState.h"
#include "XCharacter.generated.h"



UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AXCharacter();

	//ͨ��������ͻ��˴��ݷ������Ĵ�����
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//����PostInitialize��ʼ������еı���
	virtual void PostInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//��ǹ����
	void PlayFireMontage(bool bAiming);
	//�����ж���
	void PlayHitReactMontage();
	//��������
	void PlayElimMontage();
	//��������
	void PlayReloadMontage();
	//Ͷ������
	void PlayGrenadeMontage();

	//����ģ������ϵ�ת�򶯻���repnotify;
	virtual void OnRep_ReplicatedMovement() override;

	//����Ѫ��
	UFUNCTION()
		void UpdateHUDHealth();
	UFUNCTION()
		void UpdateHUDShield();
	UFUNCTION()
		void ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);


	//���ڷ������ϵ���������Ӧ
	void Elim();

	//��Ӧ����
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();

	//��ʼ��PlayerState,�������¶�Ӧ��HUD,��ҪTick��⣬��Ϊ��Ϸ�ĵ�һ֡�����ʼ��PlayerState�����޷���BeginPlay���г�ʼ����
	//�ú��������ڳ�ʼ���κ��޷��ڵ�һ֡��ʼ������
	void PollInit();

	//�ѻ�ǹ��������
	UFUNCTION(BlueprintImplementableEvent)
		void ShowSnipperScope(bool bShowScope);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//�ƶ�
	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);
	void CrouchMode();
	void RelaxToAimMode();
	void AimToRelaxMode();
	virtual void Jump() override;
	virtual void StopJumping() override;
	void ReloadWeapon();
	void Grenade();
	void DropWeapon();
	void SwapWeapon();
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float ForwardValue;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float RightValue;

	//װ������
	void EquipWeapon();

	//��׼ƫ��AO_Yaw AO_Pitch
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	UPROPERTY(BlueprintReadOnly)
		float AO_Yaw;
	float AO_Pitch;

	//ģ�����ת��
	void SimProxiesturn();

	//ת���ܼ���
	void RotateInPlace(float DeltaTime);


	FRotator StartingAimRotation;

	//��׼ƫ���µ�״̬����
	UPROPERTY(Replicated)
		ETuringInPlace TurningInPlace;

	//��������ת��ʱ��������ת�� ���ô�ֵ���в�ֵ���ת��ʱ��AO_Yaw
		float InterpAOYaw;
	////��ȡYaw�ĽǶȱ任�����޸�״̬
	UFUNCTION()
		void TurnInPlace(float DeltaTime);

	//����
		void Fireing();
		void ReFired();

	//��̫�涯��
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ElimMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* GrenadeMontage;

	void DroporDestroyWeapon(AWeaponParent* Weapon);

private:
	UPROPERTY(VisibleAnywhere, Category= Camera)
		class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* CameraComp;

	//��ɫ��ʾ���ֿռ�
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Widget,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	//��ͻ��˴����ص�������������ʾUI����
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponParent* OverlappingWeapon;
	//����Rep Notify��ȡOverlappingWeapon��״̬�ı䣬�Ӷ�������ʾUI�ĺ���
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponParent* LastWeapon);

	//ս���������
	UPROPERTY(VisibleAnywhere,Category = Combat,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCombatComponent* CombatComp;

	//�����������
	UPROPERTY(VisibleAnywhere, Category = CharacterPp)
		class UXPropertyComponent* PropertyComp;

	//Frame History Box���
	UPROPERTY(VisibleAnywhere, Category = CharacterLag)
		class ULagCompensationComponent* LagCompensationComp;

	//RPC�ͻ��˵��ã�������ִ�У�Լ���ں�����ǰ����Server
	UFUNCTION(Server,Reliable)
		void ServerEquipWeapon();
	UFUNCTION(Server, Reliable)
		void ServerSwapWeapon();

	//���ؽ�ɫ
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 100.0f;

	//�Ƿ��ܹ���ת�����������紫������������ÿһ��tick������
	bool bRotateRootBone;
	//�����жϣ�����ģ�ⲥ�����Yaw��ת�򶯻�
	float TurnThreshold = 0.5f;
	//��¼ģ���������һ֡����һ֡����ת��ֵ
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	//��¼��ֵ
	float ProxyYaw;
	//��¼�ϴ��˶�֮���Actorλ��
	float TimeSinceLastMovementReplication;

	//�������ٶȺ�������ģ������У�����ٶȴ���0��Ҫ���ò��ܹ�ת��
	float CalculateVelocity();

	//��ɫ������
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;

	//�Ƿ�����
	bool bElimmed = false;

	//����ʱ�����
	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.0f;
	void ElimTimerFinished();

	//�����ܽ�Ч��
	// 

	//��̬���������ڴ����в������� change at Runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//��̬������������ͼ�����ã��ǽ�ɫ����Ĳ���ʵ�� set on the BP,use for set the dynamic Material
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;
//ʱ����
// 
// 
	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;
	//�൱����ͼ�е�Track
	FOnTimelineFloat DissolveTrack;

	//ʱ��������
	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	//��ȡ�����ϵ�ֵ
	UFUNCTION()
		void UpdateDissolveMaterial(float Dissolve);
	//��ʼ�ܽ�
	void StartDissolve();

	//ElimBot
	UPROPERTY(EditAnywhere, Category = ElimBot)
		UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = ElimBot)
		UParticleSystemComponent* ElimBotComp;
	UPROPERTY(EditAnywhere, Category = ElimBot)
		class USoundCue* ElimBotSound;
	//ElimBot�ڿͻ��˵���ʧ������Destroyed()
	virtual void Destroyed() override;

	//Ϊ��ɫ��ʼ��PlayerState
	UPROPERTY()
		class AXBlasterPlayerState* XBlasterPlayerState;

	//Grenade component
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachGrenade;

public:	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = MoveFunc)
		bool bUnderJump = false;

	//���ص��ı�����ȡ����Ȼ�󴫸��ͻ���
	UFUNCTION()
		void SetOverlappingWeapon(AWeaponParent* Weapon);

	//�����Ƿ�װ��������Դ����ͨ����������������������Ƿ�װ��������
	UFUNCTION()
		bool GetIsEquippedWeapon();

	//��ȡ�Ƿ���׼
	UFUNCTION()
		bool GetIsAiming();

	//��������ͼ������׼ƫ�Ƶ�ֵ
	UFUNCTION()
		float GetAOYawToAnim() const;
	UFUNCTION()
		float GetAOPitchToAnim() const;

	//��������ͼ���뵱ǰװ��������
	UFUNCTION()
		AWeaponParent* GetEquippedWeapon();

	//���û�ȡ״ֵ̬��������ͼ
	ETuringInPlace GetTurninigInPlace() const;

	FVector GetHitTarget() const;

	//��ȡ��������ݸ�ս����������ڵ����ӽ�
	UFUNCTION()
	UCameraComponent* GetFollowCamera() const;

	//�����Ƿ��ܹ����ݸ�����
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE UCombatComponent* GetCombatComp() const { return CombatComp; }

	//��ȡ����������
	UXPropertyComponent* GetPropertyComp();
	//��ȡ��ҿ�����
	AXBlasterPlayerController* GetXBlasterPlayerCtr();
	//��ȡ��Ҵ�ʱ�Ƿ���װ�����Ӷ���ֹ�����е�IK
	ECombatState GetCombateState() const;

	FORCEINLINE UAnimMontage* GetRelodMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetGrenadeComp() const { return AttachGrenade; }
	FORCEINLINE bool GetbElimed() const { return bElimmed; }
	FORCEINLINE UXPropertyComponent* GetPropertyComp() const { return PropertyComp; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComp() const { return LagCompensationComp; }
	bool IsLocallyReloading();

public:
	//������Щ������������
	UPROPERTY(Replicated)
		bool bDisableGamePlay = false;

	/*Mesh ��Χ��BOX ���ڳ䵱���������˼���Ƿ����ʱ�����п�*/
	UPROPERTY(EditAnywhere)
		class UBoxComponent* headbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvisbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02box;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03box;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* backpackbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* blanketbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_rbox;

	//�洢����Box
	UPROPERTY()
		TMap<FName, UBoxComponent*> HitBoxCompMap;

};
