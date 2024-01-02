// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "XCharacter.generated.h"



UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter, public IInteractWithCrosshairInterface
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

	//开枪动画
	void PlayFireMontage(bool bAiming);
	//被击中动画
	void PlayHitReactMontage();
	////被击中动画的RPC调用
	//UFUNCTION(NetMulticast, Unreliable)
	//	void MulticastHit();

	//控制模拟机器上的转向动画，repnotify;
	virtual void OnRep_ReplicatedMovement() override;

	//更新血条
	UFUNCTION()
		void UpdateHUDHealth();
	UFUNCTION()
		void ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	UPROPERTY(BlueprintReadOnly)
		float Check = 0.0f;

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
	void CalculateAO_Pitch();
	UPROPERTY(BlueprintReadOnly)
		float AO_Yaw;
	float AO_Pitch;

	//模拟代理转向
	void SimProxiesturn();


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
		class UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* HitReactMontage;

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

	//人物属性组件
	UPROPERTY(VisibleAnywhere, Category = CharactrerPp)
		class UXPropertyComponent* PropertyComp;

	//RPC客户端调用，服务器执行，约定在函数名前加上Server
	UFUNCTION(Server,Reliable)
		void ServerEquipWeapon();

	//隐藏角色
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 100.0f;

	//是否能够旋转根骨骼，网络传播根骨骼不是每一个tick都更新
	bool bRotateRootBone;
	//控制判断，用于模拟播放替代Yaw的转向动画
	float TurnThreshold = 0.5f;
	//记录模拟机器上上一帧和这一帧的旋转差值
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	//记录差值
	float ProxyYaw;
	//记录上次运动之后的Actor位置
	float TimeSinceLastMovementReplication;

	//整理求速度函数，在模拟代码中，如果速度大于0，要设置不能够转向
	float CalculateVelocity();

	//角色控制器
	class AXBlasterPlayerController* XBlasterPlayerController;

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

	FVector GetHitTarget() const;

	//获取相机，传递给战斗组件，用于调整视角
	UCameraComponent* GetFollowCamera() const;

	//设置是否能够传递根骨骼
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
};
