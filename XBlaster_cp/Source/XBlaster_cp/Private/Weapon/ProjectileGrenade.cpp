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
	ProjectileMovementComp->MaxSpeed = InitialSpeedForGrenade;
	ProjectileMovementComp->InitialSpeed = InitialSpeedForGrenade;
	ProjectileMovementComp->SetIsReplicated(true);
	//粒子移动组件开启弹跳
	ProjectileMovementComp->bShouldBounce = true;
}

#if WITH_EDITOR
void AProjectileGrenade::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileGrenade, InitialSpeedForGrenade))
	{
		if (ProjectileMovementComp)
		{
			ProjectileMovementComp->MaxSpeed = InitialSpeedForGrenade;
			ProjectileMovementComp->InitialSpeed = InitialSpeedForGrenade;
		}
	}
}
#endif

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
