// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileActor.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Character/XCharacter.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"


// Sets default values
AProjectileActor::AProjectileActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	//子弹移动组件，在子类中创建
	//ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	//ProjectileMovementComp->bRotationFollowsVelocity = true;
	//ProjectileMovementComp->MaxSpeed = 15000.f;
	//ProjectileMovementComp->InitialSpeed = 15000.f;

}

// Called when the game starts or when spawned
void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	if (Tracer)
	{
		TracerComp = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionSphere, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}

	//绑定OnHit到OnComponentHit server
	if (HasAuthority())
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileActor::OnHit);
	}
}

//在击中时播放音效，销毁等
void AProjectileActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	////被击中对象播放击中动画
	//AXCharacter* CharacterEx = Cast<AXCharacter>(OtherActor);
	//if (CharacterEx)
	//{
	//	CharacterEx->MulticastHit();
	//}

	//摧毁,调用了Destroyed()函数
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

	//音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

