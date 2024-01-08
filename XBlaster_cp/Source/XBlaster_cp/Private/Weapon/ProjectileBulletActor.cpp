// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBulletActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBulletActor::AProjectileBulletActor()
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->MaxSpeed = 15000.f;
	ProjectileMovementComp->InitialSpeed = 15000.f;
	ProjectileMovementComp->SetIsReplicated(true);
}

void AProjectileBulletActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//Ӧ���˺�����,���Զ������˺�����
			UGameplayStatics::ApplyDamage(OtherActor, DamageBaseFloat, OwnerController, this, UDamageType::StaticClass());
		}
	}
	

	//���ڸ����OnHit�������ӵ����������Super
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
}
