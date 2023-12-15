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
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		float ZHight;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	//���ߺ��ܲ����л�
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bJump;
};
