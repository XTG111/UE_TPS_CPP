// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/XBlasterHUD.h"
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

	void ControlFire(bool bPressed);

	//RPC传递射击状态，传递是否开枪，和客户端准星位置
	UFUNCTION(Server, Reliable)
		void ServerFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//Multicast RPC
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//linetrace检测设计点
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	//添加准星 动态准星，涉及插值，所以传入参数float
	void SetHUDCrossHairs(float Deltatime);

private:
	//角色实例
	UPROPERTY(VisibleAnywhere)
		class AXCharacter* CharacterEx;

	//声明一个控制器，用控制来调用HUDPlayer::Controller -->APlayerController::GetHUD()
	class AXBlasterPlayerController* XBlasterPlayerController;
	//存储HUD
	class AXBlasterHUD* XBlasterHUD;


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

	//HUD Crosshair
	UPROPERTY(EditAnywhere)
		FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	/*
	Aiming And FOV
	*/
	//不瞄准时的默认视角
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	//视角变换插值函数
	void InterpFOV(float Deltatime);

	//自动开火的定时器
	FTimerHandle FireTime;
	//当前的开火定时器是否完成，不能这次没有走完就发射下一发子弹
	bool bCanFire = true;

	//定时器的开始和清除
	void StartFireTimer();
	void FireTimeFinished();


public:	
	void EquipWeapon(class AWeaponParent* WeaponToEquip);
};
