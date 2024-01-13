// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacterAnimInstance.h"
#include "Character/XCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/WeaponParent.h"
#include "DrawDebugHelpers.h"
#include <Kismet/KismetMathLibrary.h>
#include "Engine/SkeletalMeshSocket.h"
#include "XBlaster_cp/XTypeHeadFile/CombatState.h"

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
	EquippedWeapon = XCharacter->GetEquippedWeapon();

	//是否蹲下
	bIsCrouch = XCharacter->bIsCrouched;

	//是否瞄准
	bUnderAiming = XCharacter->GetIsAiming();

	//是否能够旋转根骨骼
	bRoatetRootBone = XCharacter->ShouldRotateRootBone();

	//是否死亡
	bElimmed = XCharacter->IsElimmed();

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

	//瞄准偏移实现转向
	TurningInPlace = XCharacter->GetTurninigInPlace();

	//左手IK
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->WeaponMesh && XCharacter->GetMesh())
	{
		//if (XCharacter->HasAuthority() && !XCharacter->IsLocallyControlled())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("BWeaponed:%d"), bWeaponEquipped);
		//}
		//获取socket位置
		LeftHandTransform = EquippedWeapon->WeaponMesh->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		XCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		////获取枪口位置
		//FTransform MuzzleTipTransform = EquippedWeapon->WeaponMesh->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		////枪口朝向
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);

		////枪口到准星点的向量
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), XCharacter->GetHitTarget(), FColor::Orange);

		//由于准星和枪口朝向有一定角度,计算右手骨骼需要旋转的量，然后在动画蓝图中利用transform bone实现旋转

		if (XCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//const USkeletalMeshSocket* RightHandSocket = EquippedWeapon->WeaponMesh->GetSocketByName(FName("hand_r"));
			//FTransform RightHandTransform = RightHandSocket->GetSocketTransform(EquippedWeapon->WeaponMesh);
			FTransform RightHandTransform = EquippedWeapon->WeaponMesh->GetSocketTransform(FName("RightHandSocket"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(XCharacter->GetHitTarget(), RightHandTransform.GetLocation());
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 20.f);
		}
	}
	
	bUseFABRIK = XCharacter->GetCombateState() == ECombatState::ECS_Unoccupied;
	bUseAimOffset = XCharacter->GetCombateState() == ECombatState::ECS_Unoccupied; //可以加上 !XCharacter->bDisableGamePlay 使得在冷却时间不能转动
	bTransformRightHand = XCharacter->GetCombateState() == ECombatState::ECS_Unoccupied; //可以加上 !XCharacter->bDisableGamePlay 使得在冷却时间不能转动
}
