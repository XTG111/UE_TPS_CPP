// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacterAnimInstance.h"
#include "Character/XCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>

void UXCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	XCharacter = Cast<AXCharacter>(TryGetPawnOwner());
}

void UXCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (XCharacter == nullptr)
	{
		XCharacter = Cast<AXCharacter>(TryGetPawnOwner());
	}

	if (XCharacter == nullptr)
	{
		return;
	}

	//获取当前z轴高度
	ZHight = XCharacter->GetActorLocation().Z;
	
	//获取速度
	FVector Velocity = XCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	//是否在空中
	//坠落判断
	bIsInAir = XCharacter->GetCharacterMovement()->IsFalling();
	//跳跃判断
	bJump = XCharacter->bUnderJump;

	//是否在加速
	bIsAccelerating = XCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;

	//是否装备武器
	bWeaponEquipped = XCharacter->GetIsEquippedWeapon();

	//是否蹲下
	bIsCrouch = XCharacter->bIsCrouched;

	//是否瞄准
	bUnderAiming = XCharacter->GetIsAiming();

	//控制持枪移动
	//鼠标的旋转 controlRotation
	FRotator AimRotation = XCharacter->GetBaseAimRotation();
	//actor Rotation
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(XCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = XCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	//AimOffset
	AO_Yaw = XCharacter->GetAOYawToAnim();
	AO_Pitch = XCharacter->GetAOPitchToAnim();
}
