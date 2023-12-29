// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileActor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectileActor::AProjectileActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	//�ӵ��ƶ����
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->MaxSpeed = 15000.f;
	ProjectileMovementComp->InitialSpeed = 15000.f;

}

// Called when the game starts or when spawned
void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	if (Tracer)
	{
		TracerComp = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionSphere, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}

	//��OnHit��OnComponentHit
	if (HasAuthority())
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileActor::OnHit);
	}
	
}

//�ڻ���ʱ������Ч�����ٵ�
void AProjectileActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	//�ݻ�,������Destroyed()����
	Destroy();
}

// Called every frame
void AProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectileActor::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	//��Ч
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

