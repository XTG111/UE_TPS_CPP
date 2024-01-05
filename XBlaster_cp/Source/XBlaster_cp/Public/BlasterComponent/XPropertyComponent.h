// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XPropertyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XBLASTER_CP_API UXPropertyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXPropertyComponent();
	friend class AXCharacter;
	//通过复制向客户端传递服务器的处理结果
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//响应伤害 对UE内置ApplayDamage的回调，需要满足一下规则形参
	UFUNCTION()
	void ReceivedDamage(float Damage, AController* InstigatorController);

	FORCEINLINE float GetMaxHealth() const { return MAXHealth; }
	FORCEINLINE float GetHealth() const { return Health; }

private:
	//Health
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MAXHealth = 100.f;

	//可复制参数

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_HealthChange, Category="Player State")
	float Health = 100.0f;
	UFUNCTION()
	void OnRep_HealthChange();	
	UPROPERTY()
		class AXCharacter* XCharacter;
};
