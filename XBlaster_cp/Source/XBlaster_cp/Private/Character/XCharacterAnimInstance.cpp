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

	//��ȡ��ǰz��߶�
	ZHight = XCharacter->GetActorLocation().Z;
	
	//��ȡ�ٶ�
	FVector Velocity = XCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	//�Ƿ��ڿ���
	//׹���ж�
	bIsInAir = XCharacter->GetCharacterMovement()->IsFalling();
	//��Ծ�ж�
	bJump = XCharacter->bUnderJump;

	//�Ƿ��ڼ���
	bIsAccelerating = XCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;

	//�Ƿ�װ������
	bWeaponEquipped = XCharacter->GetIsEquippedWeapon();

	//�Ƿ����
	bIsCrouch = XCharacter->bIsCrouched;

	//�Ƿ���׼
	bUnderAiming = XCharacter->GetIsAiming();

	//���Ƴ�ǹ�ƶ�
	//������ת controlRotation
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
