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
	//ͨ��������ͻ��˴��ݷ������Ĵ�����
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//�������������ֵ
	void HealRampUp(float DeltaTime);

	//���������shield
	void ShieldRampUp(float DeltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//��Ӧ�˺� ��UE����ApplayDamage�Ļص�����Ҫ����һ�¹����β�
	UFUNCTION()
	void ReceivedDamage(float Damage, AController* InstigatorController);

	FORCEINLINE float GetMaxHealth() const { return MAXHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxShield() const { return MAXShield; }
	FORCEINLINE float GetShield() const { return Shield; }

	//���ƽ�ɫ
	void HealCharacter(float HealAmount, float HealingTime);
	//�ָ�����
	void ShieldReplenish(float ShieldAmount, float ShealingTime);
	
	//����
	void SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed,float CrouchSpeed);

	//����
	void JumpBuff(float JumpZHight, float BuffTime);
	void SetInitialJump(float JumpZHight);

private:
	//Health
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MAXHealth = 100.f;
	//�ɸ��Ʋ���
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_HealthChange, Category="Player State")
	float Health = 100.0f;
	//ʹ�ò�������������һ������
	UFUNCTION()
	void OnRep_HealthChange(float LastHealth);

	//Shield
	UPROPERTY(EditAnywhere, Category = "Player State")
		float MAXShield = 50.f;
	//�ɸ��Ʋ���
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SheildChange, Category = "Player State")
		float Shield = 0.0f;
	//ʹ�ò�������������һ������
	UFUNCTION()
		void OnRep_SheildChange(float LastShield);

	UPROPERTY()
		class AXCharacter* XCharacter;

	UPROPERTY()
		class AXBlasterGameMode* XBlasterGameMode;

	//����Buff
	bool bHealing = false;
	float Healingrate = 0.f;
	float AmountToHeal = 0.f;

	//����buff
	bool bShield = false;
	float Shieldgrate = 0.f;
	float AmountToShield = 0.f;

	//�ٶ�Buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	//����Buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpZHight;

	//Buff���ڷ��������ͻ��˵Ĺ㲥
	UFUNCTION(NetMulticast, Reliable)
		void MulticasatSpeedBuff(float BaseSpeed, float CrouchSpeed);
	UFUNCTION(NetMulticast, Reliable)
		void MulticasatJumpBuff(float JumpZHight);
};
