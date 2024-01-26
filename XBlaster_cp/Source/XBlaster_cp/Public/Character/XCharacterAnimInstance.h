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

	//动画蓝图初始化重载
	virtual void NativeInitializeAnimation() override;

	//类似Event Tick函数
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:

	//对于私有变量，要能够被蓝图可见必须加上meta=(AllowPrivateAccess="true")
	UPROPERTY(BlueprintReadOnly,Category=Character,meta=(AllowPrivateAccess="true"))
		class AXCharacter* XCharacter;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float Speed;

	//错误，待修改，跳跃动作的判断
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float ZHight;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;

	//行走和跑步的切换
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bJump;

	//装备武器状态判断，播放装备武器的动画
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class AWeaponParent* EquippedWeapon;

	//是否蹲下
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouch;

	//是否瞄准
	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bUnderAiming;

	//控制持枪移动
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = MoveMent, meta = (AllowPrivateAccess = "true"))
		float Lean;
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	//瞄准偏移AimOffset
	UPROPERTY(BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
		float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = AimOffset, meta = (AllowPrivateAccess = "true"))
		float AO_Pitch;

	//左手IK控制
	UPROPERTY(BlueprintReadOnly, Category = LeftIK, meta = (AllowPrivateAccess = "true"))
		FTransform LeftHandTransform;

	//瞄准偏移实现转向
	UPROPERTY(BlueprintReadOnly, Category = AOTurning, meta = (AllowPrivateAccess = "true"))
		ETuringInPlace TurningInPlace;

	//右手位置，用于控制发射朝向
	UPROPERTY(BlueprintReadOnly, Category = RightHand, meta = (AllowPrivateAccess = "true"))
		FRotator RightHandRotation;

	//判断IsLocallyContollerd
	UPROPERTY(BlueprintReadOnly, Category = bLocControl, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	//控制是否能够旋转根骨骼
	UPROPERTY(BlueprintReadOnly, Category = bRotControl, meta = (AllowPrivateAccess = "true"))
		bool bRoatetRootBone;
	
	//控制是否死亡
	UPROPERTY(BlueprintReadOnly, Category = Elimm, meta = (AllowPrivateAccess = "true"))
		bool bElimmed;

	//是否禁用IK
	UPROPERTY(BlueprintReadOnly, Category = ControlIK, meta = (AllowPrivateAccess = "true"))
		bool bUseFABRIK;
	//换弹时禁止瞄准偏移
	UPROPERTY(BlueprintReadOnly, Category = ContorlAimOffset, meta = (AllowPrivateAccess = "true"))
		bool bUseAimOffset;
	//换弹时不旋转右手
	UPROPERTY(BlueprintReadOnly, Category = ContorlAimOffset, meta = (AllowPrivateAccess = "true"))
		bool bTransformRightHand;

	//是否持旗子
	UPROPERTY(BlueprintReadOnly, Category = HoldTheFlag, meta = (AllowPrivateAccess = "true"))
		bool bHoldingTheFlag;
};
