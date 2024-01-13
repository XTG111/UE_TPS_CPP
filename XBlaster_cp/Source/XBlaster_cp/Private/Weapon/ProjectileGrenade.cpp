// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMeshComp"));
	ProjectileMeshComp->SetupAttachment(RootComponent);
	ProjectileMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->MaxSpeed = 1500.f;
	ProjectileMovementComp->InitialSpeed = 1500.f;
	ProjectileMovementComp->SetIsReplicated(true);
	//粒子移动组件开启弹跳
	ProjectileMovementComp->bShouldBounce = true;
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	ProjectileMovementComp->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceCue, GetActorLocation(), 0.5f, 0.2f);
	}
}
