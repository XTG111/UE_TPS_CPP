// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp/PickUpActorParent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
APickUpActorParent::APickUpActorParent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OverlapShpere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapShere"));
	OverlapShpere->SetupAttachment(RootComponent);
	OverlapShpere->SetSphereRadius(150.f);
	OverlapShpere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapShpere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapShpere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapShpere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	PickUpMesh->SetupAttachment(OverlapShpere);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));

	PickUpMesh->SetRenderCustomDepth(true);
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComp"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void APickUpActorParent::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickUpActorParent::BindOverlapTimerFinished, BindOverlapTime);
	}
}

// Called every frame
void APickUpActorParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PickUpMesh)
	{
		PickUpMesh->AddWorldRotation(FRotator(0.f, BaseTureRate * DeltaTime, 0.f));
	}

}

void APickUpActorParent::Destroyed()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	Super::Destroyed();

	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickUpSound,
			GetActorLocation()
		);
	}
}

void APickUpActorParent::OnShpereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

//绑定事件在0.25s后才会生效，所以这段时间ACTOR可以生成，且不会被销毁
void APickUpActorParent::BindOverlapTimerFinished()
{
	//Bind Overlap
	OverlapShpere->OnComponentBeginOverlap.AddDynamic(this, &APickUpActorParent::OnShpereOverlap);
	GetWorldTimerManager().ClearTimer(BindOverlapTimer);
}

