// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletShellActor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABulletShellActor::ABulletShellActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BulletShellComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellComp"));
	SetRootComponent(BulletShellComp);

	//������ײ
	BulletShellComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletShellComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	//��������
	BulletShellComp->SetSimulatePhysics(true);
	BulletShellComp->SetEnableGravity(true);

	//��Ҫ�������ǵĸ�����ײ --> simulation generates hit events
	BulletShellComp->SetNotifyRigidBodyCollision(true);

	//
	ShellEjectionImpulse = 10.0f;

}

// Called when the game starts or when spawned
void ABulletShellActor::BeginPlay()
{
	Super::BeginPlay();
	//��ӳ���
	BulletShellComp->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

	BulletShellComp->OnComponentHit.AddDynamic(this, &ABulletShellActor::OnHit);
	
}

// Called every frame
void ABulletShellActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletShellActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpilse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	//�ȴ�һ���¼���������ٺ���
	GetWorldTimerManager().SetTimer(DestroyTimeControl, this, &ABulletShellActor::ClearTimerHandle_BulletShell, 1.0f, true);
}

void ABulletShellActor::ClearTimerHandle_BulletShell()
{
	GetWorldTimerManager().ClearTimer(DestroyTimeControl);
	Destroy();
}

