// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XBLASTER_CP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//通过复制向客户端传递服务器的处理结果 EquippedWeapon
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//友元类，访问私有变量
	friend class AXCharacter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//在这个组件中设置是否在瞄准，方便RPC的使用
	UFUNCTION()
		void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	//客户端上持枪的时候改变控制旋转方式
	UFUNCTION()
		void OnRep_EquippedWeapon();

	//攻击
	void IsFired(bool bPressed);

	//RPC传递射击状态
	UFUNCTION(Server, Reliable)
		void ServerFire(bool bPressed);

	//Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed);

	//linetrace检测设计点
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

private:
	//角色实例
	UPROPERTY(VisibleAnywhere)
		class AXCharacter* CharacterEx;


	//装备上的武器实例
	//EquippedWeapon用来判断是否装备武器，从而切换动画，需要赋值给客户端
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon,VisibleAnywhere)
	class AWeaponParent* EquippedWeapon;

	//瞄准状态
	UPROPERTY(Replicated, VisibleAnywhere)
		bool bUnderAiming;

	UPROPERTY(VisibleAnywhere)
		float BaseWalkSpeed;
	UPROPERTY(VisibleAnywhere)
		float AimWalkSpeed;

	//攻击
	UPROPERTY(Replicated)
		bool bFired = false;
	//目标攻击点
	FVector HitTarget;

public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);
};
