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

	//client OnHit�¼�������ֻ�ڷ���������Ӧ�ˣ����������пͻ�������Ӧ
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
	//�������Ǹı���OnHit�¼��Ĵ���λ�ã��������з����������Դ��������˺��ļ���ֻ��Ҫ�ڷ���������ɷ������пͻ��˶�����Ӧ
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

//������ʱ��Ҫ���ٵģ������ǵȴ�3s
void AProjectileRocket::SelfPlayDestroy()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	//��Ч
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	//mesh����ʧ
	if (RocketMeshComp)
	{
		RocketMeshComp->SetVisibility(false);
	}
	//������ײ
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//����������Ч
	if (TrailSystemComp && TrailSystemComp->GetSystemInstance())
	{
		TrailSystemComp->GetSystemInstance()->Deactivate();
	}
	//����Rocket���е�����
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

