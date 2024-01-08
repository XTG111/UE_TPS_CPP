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

	//��дDestroyed()��������Ϊ��Rocket�����ӳ�3s�ŵ���Destroy()
	virtual void Destroyed() override;

	void SelfPlayDestroy();
	
protected:
	virtual void BeginPlay() override;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit) override;

	//������ڿ��е�����
	UPROPERTY(EditAnywhere,Category = "Sound")
		class USoundCue* RocketLoopSound;
	UPROPERTY()
		UAudioComponent* RocketLoopComp;
	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundAttenuation* LoopingSoundAtt;

	//��ʱ����������
	void DestroyTimerFinished();

	//NiagaraSystem,��������Niagara
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	//NiagaraComponent �����洢���ɵ�����
	UPROPERTY(EditAnywhere)
		class UNiagaraComponent* TrailSystemComp;

	//����ӵ���ר���˶����
	UPROPERTY(VisibleAnywhere)
		class URocketMovementComponent* RocketMovementComp;

private:
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* RocketMeshComp;

	//��ʱ��������TrailSystem��ʧ�ٴݻ�Rocket
	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;
};
