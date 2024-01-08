// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileActor.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API AProjectileRocket : public AProjectileActor
{
	GENERATED_BODY()
public:
	AProjectileRocket();

	//重写Destroyed()函数，因为在Rocket中是延迟3s才调用Destroy()
	virtual void Destroyed() override;

	void SelfPlayDestroy();
	
protected:
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit) override;

	//火箭弹在空中的声音
	UPROPERTY(EditAnywhere,Category = "Sound")
		class USoundCue* RocketLoopSound;
	UPROPERTY()
		UAudioComponent* RocketLoopComp;
	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundAttenuation* LoopingSoundAtt;

	//定时器结束调用
	void DestroyTimerFinished();

	//NiagaraSystem,用于生成Niagara
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//NiagaraComponent 用来存储生成的粒子
	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* TrailSystemComp;

	//火箭子弹的专属运动组件
	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMovementComp;

private:
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RocketMeshComp;

	//定时器，控制TrailSystem消失再摧毁Rocket
	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;
};
