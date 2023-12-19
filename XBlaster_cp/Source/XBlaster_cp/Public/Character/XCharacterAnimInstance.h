// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
};
