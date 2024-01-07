// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMeshComp"));
	RocketMeshComp->SetupAttachment(RootComponent);
	RocketMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	APawn* FiringPawn =  GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			//Damage 5.f~DamageBaseFloat 
			//Radius, 200~500  <200
			UGameplayStatics::ApplyRadialDamageWithFalloff(this,
				DamageBaseFloat,
				5.f,
				GetActorLocation(),
				200.f,
				500.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController);

		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
}

