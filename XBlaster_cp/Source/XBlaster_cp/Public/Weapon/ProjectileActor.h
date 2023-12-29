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


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	class UParticleSystemComponent* TracerComp;

	//击中特效
	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
		class USoundCue* ImpactSound;


};
