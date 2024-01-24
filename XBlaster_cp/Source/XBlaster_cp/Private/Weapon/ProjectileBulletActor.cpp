// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBulletActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Character/XCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BlasterComponent/LagCompensationComponent.h"
#include "PlayerController/XBlasterPlayerController.h"

AProjectileBulletActor::AProjectileBulletActor()
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->MaxSpeed = InitialSpeedForBullet;
	ProjectileMovementComp->InitialSpeed = InitialSpeedForBullet;
	ProjectileMovementComp->SetIsReplicated(true);
}

#if WITH_EDITOR
void AProjectileBulletActor::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBulletActor, InitialSpeedForBullet))
	{
		if (ProjectileMovementComp)
		{
			ProjectileMovementComp->MaxSpeed = InitialSpeedForBullet;
			ProjectileMovementComp->InitialSpeed = InitialSpeedForBullet;
		}
	}
}
#endif

void AProjectileBulletActor::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBulletActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	AXCharacter* OwnerCharacter = Cast<AXCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		//AController * OwnerController = OwnerCharacter->Controller;
		AXBlasterPlayerController* OwnerController = Cast<AXBlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			//bool bCauseAuthDamage = !bUseServerSideRewind || OwnerCharacter->IsLocallyControlled();
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				bool bHeadShot = Hit.BoneName.ToString() == FString("head");

				const float DamageToCause = bHeadShot ? HeadShotDamage : DamageBaseFloat;
				//应用伤害函数,将自动调用伤害设置
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				//由于父类的OnHit会销毁子弹，所以最后Super
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
				return;
			}
			//客户端调用ServerRPC实现打击
			AXCharacter* HitCharacter = Cast<AXCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComp() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComp()->ServerProjectileScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetSeverTime() - OwnerController->SingleTripTime
				);
			}
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
}


