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
	//处理子弹命中后的销毁等操作,就是蓝图中的OnHit事件
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

	//子弹的运动粒子特效的添加
	UPROPERTY(EditAnywhere)
		class UParticleSystem* Tracer;
	UPROPERTY()
		class UParticleSystemComponent* TracerComp;

	//击中特效
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
		float DamageBaseFloat = 10.f;

	//NiagaraSystem,用于生成Niagara
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//NiagaraComponent 用来存储生成的粒子
	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* TrailSystemComp;
	void SpawnTrailSystem();

	//定时器结束调用
	void DestroyTimerFinished();
	//调用开启定时器
	void StartDestroyTimer();

	//子弹网格体,只在实际需要的类中创建比如火箭弹
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* ProjectileMeshComp;

	//爆炸伤害
	void ExplodeDamage();

	//伤害范围
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
	//定时器，控制TrailSystem消失再摧毁Rocket
	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;
};
