// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/SnipperWeaponParent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/XCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterComponent/CombatComponent.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "BlasterComponent/LagCompensationComponent.h"

void ASnipperWeaponParent::Fire(const FVector& HitTarget)
{
	//ֱ�ӵ������������еĿ�����
	AWeaponParent::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	AXCharacter* OwnerCharacter = Cast<AXCharacter>(OwnerPawn);
	bAiming = OwnerCharacter->GetCombatComp()->GetbAiming();

	//��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName("MuzzleFlash");
	//Controllerֻ�����ڱ��أ�������ģ��Actor�Ļ����϶��ǲ����ڵģ�
	//�����������������Χ�ж�ʹ����&& InstigatorController ��ô���������������Ϲ۲ⲻ��������Чֻ�����˺���Ӧ
	if (MuzzleFlashSocket)
	{
		FTransform SockertTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
		//��ǹ�ڳ���
		FVector Start = SockertTransform.GetLocation();

		FHitResult FireHit;

		SnipperTraceHit(Start, HitTarget, FireHit);

		//�˺�����
		AXCharacter* HitCharacter = Cast<AXCharacter>(FireHit.GetActor());
		if (HitCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewide || OwnerPawn->IsLocallyControlled();
			//����Ƿ������ϵ�Ȩ��Actor��ǹ
			if (HasAuthority() && bCauseAuthDamage)
			{
				//�˺��ж��ڷ������Ͻ���
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			//���ֻ�ǿͻ����ϵ�Actor��ǹ���ҿ�����ServeRewide
			else if (!HasAuthority() && bUseServerSideRewide)
			{
				XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetOwner()) : XCharacter;
				XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(InstigatorController) : XBlasterPlayerController;
				if (XCharacter && XBlasterPlayerController && XCharacter->GetLagCompensationComp() && XCharacter->IsLocallyControlled())
				{
					XCharacter->GetLagCompensationComp()->ServerScoreRequest(
						HitCharacter,
						Start,
						HitTarget,
						XBlasterPlayerController->GetSeverTime() - XBlasterPlayerController->SingleTripTime,
						this
					);
				}
			}
		}
		//���߼���������к����������Ч
		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticle,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		//���߼�������Ļ�����Ч
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint,
				0.5f,
				FMath::FRandRange(-0.5f, 0.5f)
			);
		}
	}

}

void ASnipperWeaponParent::SnipperTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	//�Ƿ���ɢ���������ˣ��͵��������Ǹ�����������ĺ����������������յ����׼��λ��
	FVector End =  !bAiming ? TraceEndWithScatter(HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
	if (World)
	{
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (BeamComp)
			{
				BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
