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

	//声明一个定时器
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

	//各个类型添加冲量的程度
	UPROPERTY(EditAnywhere)
		float ShellEjectionImpulse;

	//落地音效
	UPROPERTY(EditAnywhere)
		class USoundCue* ShellSound;

protected:

	//当弹壳掉落到地上是需要销毁
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit);

	//销毁弹壳并清除定时器
	void ClearTimerHandle_BulletShell();

};
