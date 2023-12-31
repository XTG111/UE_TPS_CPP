// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Weapon/RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMeshComp"));
	RocketMeshComp->SetupAttachment(RootComponent);
	RocketMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComp = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComp"));
	RocketMovementComp->bRotationFollowsVelocity = true;
	RocketMovementComp->MaxSpeed = 15000.f;
	RocketMovementComp->InitialSpeed = 15000.f;
	RocketMovementComp->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	//client OnHit事件不再是只在服务器上响应了，而是在所有客户端上响应
	if (!HasAuthority())
	{
		CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	//Spawn NiagaraSystem;
	if (TrailSystem)
	{
		TrailSystemComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

	//Spawn LoopingSound
	if (RocketLoopSound && LoopingSoundAtt)
	{
		RocketLoopComp = UGameplayStatics::SpawnSoundAttached(
			RocketLoopSound, 
			GetRootComponent(),
			FName(), 
			GetActorLocation(), 
			EAttachLocation::KeepWorldPosition, 
			false,
			1.0f,
			1.f,
			0.f,
			LoopingSoundAtt,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	APawn* FiringPawn =  GetInstigator();
	//由于我们改变了OnHit事件的处理位置，现在所有服务器都可以处理，但是伤害的计算只需要在服务器上完成否则所有客户端都会响应
	if (FiringPawn&&HasAuthority())
	{
		if (OtherActor == GetOwner()) 
		{
			return;
		}
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
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);
	SelfPlayDestroy();
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
}

//当击中时需要销毁的，而不是等待3s
void AProjectileRocket::SelfPlayDestroy()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	//音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	//mesh的消失
	if (RocketMeshComp)
	{
		RocketMeshComp->SetVisibility(false);
	}
	//消除碰撞
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//消除粒子特效
	if (TrailSystemComp && TrailSystemComp->GetSystemInstance())
	{
		TrailSystemComp->GetSystemInstance()->Deactivate();
	}
	//消除Rocket飞行的声音
	if (RocketLoopComp && RocketLoopComp->IsPlaying())
	{
		RocketLoopComp->Stop();
	}
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
}

