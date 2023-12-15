// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacterAnimInstance.h"
#include "Character/XCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	bIsInAir = XCharacter->GetCharacterMovement()->IsFalling();
	bJump = XCharacter->bUnderJump;

	//是否在加速
	bIsAccelerating = XCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false;
}
