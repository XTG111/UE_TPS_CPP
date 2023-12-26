// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "XCharacter.generated.h"



UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AXCharacter();

	//通过复制向客户端传递服务器的处理结果
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//利用PostInitialize初始化组件中的变量
	virtual void PostInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//移动
	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);
	void CrouchMode();
	void RelaxToAimMode();
	void AimToRelaxMode();
	virtual void Jump() override;
	virtual void StopJumping() override;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float ForwardValue;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float RightValue;

	//装备武器
	void EquipWeapon();

	//瞄准偏移AO_Yaw AO_Pitch
	void AimOffset(float DeltaTime);
	UPROPERTY(BlueprintReadOnly,Replicated)
		float AO_Yaw;
	UFUNCTION(Server, Reliable)
		void AOYawTrans(float DeltaTime);
	float AO_Pitch;
	FRotator StartingAimRotation;

	//瞄准偏移下的状态控制
	UPROPERTY(Replicated)
		ETuringInPlace TurningInPlace;

	//用于设置转动时根骨骼的转动 利用此值进行插值求得转动时的AO_Yaw
		float InterpAOYaw;
	////获取Yaw的角度变换，来修改状态
	UFUNCTION()
		void TurnInPlace(float DeltaTime);

	//攻击
	void Fireing();
	void ReFired();
	//蒙太奇动画
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* EquipFireMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* AimFireMontage;


private:
	UPROPERTY(VisibleAnywhere, Category= Camera)
		class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* CameraComp;

	//角色显示文字空间
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Widget,meta=(AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	//向客户端传递重叠的武器，以显示UI界面
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponParent* OverlappingWeapon;
	//利用Rep Notify获取OverlappingWeapon的状态改变，从而调用显示UI的函数
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponParent* LastWeapon);

	//战斗组件设置
	UPROPERTY(VisibleAnywhere,Category = Combat)
		class UCombatComponent* CombatComp;

	//RPC客户端调用，服务器执行，约定在函数名前加上Server
	UFUNCTION(Server,Reliable)
		void ServerEquipWeapon();

public:	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = MoveFunc)
		bool bUnderJump = false;

	//将重叠的变量获取到，然后传给客户端
	UFUNCTION()
		void SetOverlappingWeapon(AWeaponParent* Weapon);

	//设置是否装备，动画源码中通过调用这个函数，来设置是否装备了武器
	UFUNCTION()
		bool GetIsEquippedWeapon();

	//获取是否瞄准
	UFUNCTION()
		bool GetIsAiming();

	//给动画蓝图传入瞄准偏移的值
	UFUNCTION()
		float GetAOYawToAnim() const;
	UFUNCTION()
		float GetAOPitchToAnim() const;

	//给动画蓝图传入当前装备的武器
	UFUNCTION()
		AWeaponParent* GetEquippedWeapon();

	//设置获取状态值到动画蓝图
	ETuringInPlace GetTurninigInPlace() const;

	void PlayFireMontage(bool bAiming);
};
