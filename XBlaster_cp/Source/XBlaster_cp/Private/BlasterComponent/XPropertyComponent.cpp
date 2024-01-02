// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/XPropertyComponent.h"
#include "Net/UnrealNetWork.h"
#include "Character/XCharacter.h"

// Sets default values for this component's properties
UXPropertyComponent::UXPropertyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
	// ...
}

void UXPropertyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(UXPropertyComponent, Health);
}


// Called when the game starts
void UXPropertyComponent::BeginPlay()
{
	Super::BeginPlay();
	XCharacter = Cast<AXCharacter>(GetOwner());

	//
}


// Called every frame
void UXPropertyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXPropertyComponent::ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MAXHealth);
	XCharacter->UpdateHUDHealth();
	//将受击动画的播放改到这里，降低RPC调用的负担
	XCharacter->PlayHitReactMontage();
	
}

void UXPropertyComponent::OnRep_HealthChange()
{
	XCharacter->UpdateHUDHealth();
	XCharacter->PlayHitReactMontage();
}

