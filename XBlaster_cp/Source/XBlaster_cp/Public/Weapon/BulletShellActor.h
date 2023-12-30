// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShellActor.generated.h"

UCLASS()
class XBLASTER_CP_API ABulletShellActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletShellActor();

	//����һ����ʱ��
	FTimerHandle DestroyTimeControl;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* BulletShellComp;

	//����������ӳ����ĳ̶�
	UPROPERTY(EditAnywhere)
		float ShellEjectionImpulse;

	//�����Ч
	UPROPERTY(EditAnywhere)
		class USoundCue* ShellSound;

protected:

	//�����ǵ��䵽��������Ҫ����
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit);

	//���ٵ��ǲ������ʱ��
	void ClearTimerHandle_BulletShell();

};
