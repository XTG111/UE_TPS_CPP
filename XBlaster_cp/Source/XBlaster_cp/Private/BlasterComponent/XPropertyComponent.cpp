// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponent/XPropertyComponent.h"
#include "Net/UnrealNetWork.h"
#include "Character/XCharacter.h"
#include "GameMode/XBlasterGameMode.h"
#include "PlayerController/XBlasterPlayerController.h"

// Sets default values for this component's properties
UXPropertyComponent::UXPropertyComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
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

void UXPropertyComponent::ReceivedDamage(float Damage, AController* InstigatorController)
{
	//����ɫ�����󣬾Ͳ����ٴν����˺�
	if (XCharacter->GetbElimed()) return;

	Health = FMath::Clamp(Health - Damage, 0.f, MAXHealth);
	XCharacter->UpdateHUDHealth();
	//���ܻ������Ĳ��Ÿĵ��������RPC���õĸ���
	XCharacter->PlayHitReactMontage();

	if (Health == 0.0f)
	{
		AXBlasterGameMode* XBlasterGameMode = GetWorld()->GetAuthGameMode<AXBlasterGameMode>();
		if (XBlasterGameMode && XCharacter)
		{	
			//XCharacter->XBlasterPlayerController = XCharacter->XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XCharacter->XBlasterPlayerController;
			AXBlasterPlayerController* AttackerContorller = Cast<AXBlasterPlayerController>(InstigatorController);
			XBlasterGameMode->PlayerEliminated(XCharacter, XCharacter->GetXBlasterPlayerCtr(), AttackerContorller);
		}
	}
}

void UXPropertyComponent::OnRep_HealthChange()
{
	XCharacter->UpdateHUDHealth();
	XCharacter->PlayHitReactMontage();
}

