// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "XCharacter.generated.h"



UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter
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
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float ForwardValue;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float RightValue;

	//װ������
	void EquipWeapon();

	//��׼ƫ��AO_Yaw AO_Pitch
	void AimOffset(float DeltaTime);
	UPROPERTY(BlueprintReadOnly,Replicated)
		float AO_Yaw;
	UFUNCTION(Server, Reliable)
		void AOYawTrans(float DeltaTime);
	float AO_Pitch;
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
		class UAnimMontage* EquipFireMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* AimFireMontage;


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
	UPROPERTY(VisibleAnywhere,Category = Combat)
		class UCombatComponent* CombatComp;

	//RPC�ͻ��˵��ã�������ִ�У�Լ���ں�����ǰ����Server
	UFUNCTION(Server,Reliable)
		void ServerEquipWeapon();

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

	void PlayFireMontage(bool bAiming);
};
