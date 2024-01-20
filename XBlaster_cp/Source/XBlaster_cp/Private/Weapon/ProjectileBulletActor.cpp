// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBulletActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBulletActor::AProjectileBulletActor()
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true;
	ProjectileMovementComp->MaxSpeed = InitialSpeedForBullet;
	ProjectileMovementComp->InitialSpeed = InitialSpeedForBullet;
	ProjectileMovementComp->SetIsReplicated(true);
}

#if WITH_EDITOR
void AProjectileBulletActor::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBulletActor, InitialSpeedForBullet))
	{
		if (ProjectileMovementComp)
		{
			ProjectileMovementComp->MaxSpeed = InitialSpeedForBullet;
			ProjectileMovementComp->InitialSpeed = InitialSpeedForBullet;
		}
	}
}
#endif

void AProjectileBulletActor::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;
	PathParams.bTraceWithCollision = true;
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeedForBullet;
	PathParams.MaxSimTime = 4.f; //飞行时间
	PathParams.ProjectileRadius = 5.f;//绘制半径
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation(); //预测起点
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PathResult;

	//预测轨迹
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}

void AProjectileBulletActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//应用伤害函数,将自动调用伤害设置
			UGameplayStatics::ApplyDamage(OtherActor, DamageBaseFloat, OwnerController, this, UDamageType::StaticClass());
		}
	}
	

	//由于父类的OnHit会销毁子弹，所以最后Super
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpilse, Hit);
}


