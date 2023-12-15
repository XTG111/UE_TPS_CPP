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

	//动画蓝图初始化重载
	virtual void NativeInitializeAnimation() override;

	//类似Event Tick函数
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:

	//对于私有变量，要能够被蓝图可见必须加上meta=(AllowPrivateAccess="true")
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
		class AXCharacter* XCharacter;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		float ZHight;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	//行走和跑步的切换
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		bool bJump;
};
