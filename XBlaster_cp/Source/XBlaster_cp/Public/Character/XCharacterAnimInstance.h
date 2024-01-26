// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "XCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UXCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	//������ͼ��ʼ������
	virtual void NativeInitializeAnimation() override;

	//����Event Tick����
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:

	//����˽�б�����Ҫ�ܹ�����ͼ�ɼ��������meta=(AllowPrivateAccess="true")
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
		class AXCharacter* XCharacter;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float Speed;

	//���󣬴��޸ģ���Ծ�������ж�
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float ZHight;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	//���ߺ��ܲ����л�
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bJump;

	//װ������״̬�жϣ�����װ�������Ķ���
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class AWeaponParent* EquippedWeapon;

	//�Ƿ����
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouch;

	//�Ƿ���׼
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bUnderAiming;

	//���Ƴ�ǹ�ƶ�
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float Lean;
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	//��׼ƫ��AimOffset
	UPROPERTY(BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
		float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
		float AO_Pitch;

	//����IK����
	UPROPERTY(BlueprintReadOnly, Category = LeftIK, meta = (AllowPrivateAccess = "true"))
		FTransform LeftHandTransform;

	//��׼ƫ��ʵ��ת��
	UPROPERTY(BlueprintReadOnly, Category = AOTurning, meta = (AllowPrivateAccess = "true"))
		ETuringInPlace TurningInPlace;

	//����λ�ã����ڿ��Ʒ��䳯��
	UPROPERTY(BlueprintReadOnly, Category = RightHand, meta = (AllowPrivateAccess = "true"))
		FRotator RightHandRotation;

	//�ж�IsLocallyContollerd
	UPROPERTY(BlueprintReadOnly, Category = bLocControl, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	//�����Ƿ��ܹ���ת������
	UPROPERTY(BlueprintReadOnly, Category = bRotControl, meta = (AllowPrivateAccess = "true"))
		bool bRoatetRootBone;
	
	//�����Ƿ�����
	UPROPERTY(BlueprintReadOnly, Category = Elimm, meta = (AllowPrivateAccess = "true"))
		bool bElimmed;

	//�Ƿ����IK
	UPROPERTY(BlueprintReadOnly, Category = ControlIK, meta = (AllowPrivateAccess = "true"))
		bool bUseFABRIK;
	//����ʱ��ֹ��׼ƫ��
	UPROPERTY(BlueprintReadOnly, Category = ContorlAimOffset, meta = (AllowPrivateAccess = "true"))
		bool bUseAimOffset;
	//����ʱ����ת����
	UPROPERTY(BlueprintReadOnly, Category = ContorlAimOffset, meta = (AllowPrivateAccess = "true"))
		bool bTransformRightHand;

	//�Ƿ������
	UPROPERTY(BlueprintReadOnly, Category = HoldTheFlag, meta = (AllowPrivateAccess = "true"))
		bool bHoldingTheFlag;
};
