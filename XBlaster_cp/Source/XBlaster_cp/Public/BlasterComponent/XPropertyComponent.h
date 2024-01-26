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

	//按比例提高生命值
	void HealRampUp(float DeltaTime);

	//按比例提高shield
	void ShieldRampUp(float DeltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//响应伤害 对UE内置ApplayDamage的回调，需要满足一下规则形参
	UFUNCTION()
	void ReceivedDamage(float Damage, AController* InstigatorController);

	FORCEINLINE float GetMaxHealth() const { return MAXHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxShield() const { return MAXShield; }
	FORCEINLINE float GetShield() const { return Shield; }

	//治疗角色
	void HealCharacter(float HealAmount, float HealingTime);
	//恢复护盾
	void ShieldReplenish(float ShieldAmount, float ShealingTime);
	
	//加速
	void SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed,float CrouchSpeed);

	//弹跳
	void JumpBuff(float JumpZHight, float BuffTime);
	void SetInitialJump(float JumpZHight);

private:
	//Health
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MAXHealth = 100.f;
	//可复制参数
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_HealthChange, Category="Player State")
	float Health = 100.0f;
	//使用参数决定播放哪一个动画
	UFUNCTION()
	void OnRep_HealthChange(float LastHealth);

	//Shield
	UPROPERTY(EditAnywhere, Category = "Player State")
		float MAXShield = 50.f;
	//可复制参数
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SheildChange, Category = "Player State")
		float Shield = 0.0f;
	//使用参数决定播放哪一个动画
	UFUNCTION()
		void OnRep_SheildChange(float LastShield);

	UPROPERTY()
		class AXCharacter* XCharacter;

	UPROPERTY()
		class AXBlasterGameMode* XBlasterGameMode;

	//治疗Buff
	bool bHealing = false;
	float Healingrate = 0.f;
	float AmountToHeal = 0.f;

	//护盾buff
	bool bShield = false;
	float Shieldgrate = 0.f;
	float AmountToShield = 0.f;

	//速度Buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	//弹跳Buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpZHight;

	//Buff用于服务器到客户端的广播
	UFUNCTION(NetMulticast, Reliable)
		void MulticasatSpeedBuff(float BaseSpeed, float CrouchSpeed);
	UFUNCTION(NetMulticast, Reliable)
		void MulticasatJumpBuff(float JumpZHight);
};
