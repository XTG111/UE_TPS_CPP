// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

UCLASS()
class XBLASTER_CP_API AProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	//�����ӵ����к�����ٵȲ���,������ͼ�е�OnHit�¼�
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

public:

	UPROPERTY(EditAnywhere)
		class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovementComp;

	//�ӵ����˶�������Ч�����
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;
	UPROPERTY()
		class UParticleSystemComponent* TracerComp;

	//������Ч
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
		float DamageBaseFloat = 10.f;

	//NiagaraSystem,��������Niagara
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//NiagaraComponent �����洢���ɵ�����
	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* TrailSystemComp;
	void SpawnTrailSystem();

	//��ʱ����������
	void DestroyTimerFinished();
	//���ÿ�����ʱ��
	void StartDestroyTimer();

	//�ӵ�������,ֻ��ʵ����Ҫ�����д�����������
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* ProjectileMeshComp;

	//��ը�˺�
	void ExplodeDamage();

	//�˺���Χ
	UPROPERTY(EditAnywhere)
		float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
		float DamageOuterRadius = 500.f;

	/*use for server-rewind*/
	UPROPERTY(EditAnywhere)
		bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere, Category = "InitialSpeed")
		float InitialSpeedForBullet = 15000.f;
	UPROPERTY(EditAnywhere, Category = "InitialSpeed")
		float InitialSpeedForGrenade = 1500.f;
	UPROPERTY(EditAnywhere, Category = "InitialSpeed")
		float InitialSpeedForRocket = 15000.f;

private:
	//��ʱ��������TrailSystem��ʧ�ٴݻ�Rocket
	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;
};
