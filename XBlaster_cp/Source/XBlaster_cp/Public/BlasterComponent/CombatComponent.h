// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/XBlasterHUD.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "XBlaster_cp/XTypeHeadFile/CombatState.h"
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

	//结束装弹动画，修改状态为空CombatState
	UFUNCTION(BlueprintCallable)
		void FinishingReloading();

	//动画通知 当交换武器动画结束
	UFUNCTION(BlueprintCallable)
		void FinishSwap();

	UFUNCTION(BlueprintCallable)
		void FinishSwapSecondWeapon();

	//攻击函数
	void IsFired(bool bPressed);

	//动画通知调用统计霰弹枪此时的子弹数
	UFUNCTION(BlueprintCallable)
		void ShotGunShellReload();
	UFUNCTION(BlueprintCallable)
		void ShotGunNoFire();

	//动画通知调用该函数，执行投掷结束后状态的切换
	UFUNCTION(BlueprintCallable)
		void ThrowGrenadeFinished();
	//扔出手雷
	UFUNCTION(BlueprintCallable)
		void LaunchGrenade();
	//ServerRPC将投掷目标点传给服务器
	UFUNCTION(Server, Reliable)
		void ServerLauncherGrenade(const FVector_NetQuantize& Target);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//在这个组件中设置是否在瞄准，方便RPC的使用
	UFUNCTION()
		void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	//客户端上持枪的时候改变控制旋转方式
	//控制主武器
	UFUNCTION()
		void OnRep_EquippedWeapon();

	//控制副武器
	UFUNCTION()
		void OnRep_SecondWeapon();

	//判断能否攻击
	void ControlFire(bool bPressed);
	//对不同开火状态的武器编写各自的开火函数
	void FireProjectileWeapon(bool bPressed);
	void FireHitScanWeapon(bool bPressed);
	void FireShotGunWeapon(bool bPressed);

	//RPC传递霰弹枪的命中目标到服务器
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerShotGunFire(bool bPressed, const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastShotGunFire(bool bPressed, const TArray<FVector_NetQuantize>& TraceHitTargets);

	//RPC传递射击状态，传递是否开枪，和客户端准星位置Call from Client Do in the server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget, float FireDelay);
	//Multicast RPC Call from Server
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(bool bPressed, const FVector_NetQuantize& TraceHitTarget);

	//本地播放开火的动画，降低延迟影响
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	//霰弹枪的本地开火控制，用于统计命中点
	void LocalShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	////RPC传递换弹动画
	UFUNCTION(Server, Reliable)
		void ServerReload();

	//在服务器和客户端上所有的共享动作
	void HandleReload();

	//linetrace检测设计点
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	//添加准星 动态准星，涉及插值，所以传入参数float
	void SetHUDCrossHairs(float Deltatime);

	//计算重新装弹
	int32 AmmountToReload();

	//手雷的UI
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 GrenadeAmount = 4;
	UFUNCTION()
		void OnRep_Grenades();
	UPROPERTY(EditAnywhere)
		int32 MaxGrenade = 4;

	void UpdateHUDGrenade();

	//丢弃
	UFUNCTION(Server, Reliable)
		void ServerDropWeapon();

	//投掷
	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
		void ServerThrowGrenade();

	//投掷手雷的类
	UPROPERTY(EditAnywhere)
		TSubclassOf<class AProjectileActor> GrenadeClass;

private:
	//角色实例
	UPROPERTY(VisibleAnywhere)
		AXCharacter* CharacterEx;

	//声明一个控制器，用控制来调用HUDPlayer::Controller -->APlayerController::GetHUD()
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;
	//存储HUD
	UPROPERTY()
		class AXBlasterHUD* XBlasterHUD;


	//装备上的武器实例
	//EquippedWeapon用来判断是否装备武器，从而切换动画，需要赋值给客户端
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon,VisibleAnywhere)
		class AWeaponParent* EquippedWeapon;

	//副武器
	UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon, VisibleAnywhere)
		AWeaponParent* SecondWeapon;

	//瞄准状态
	UPROPERTY(ReplicatedUsing = OnRep_UnderAming, VisibleAnywhere)
		bool bUnderAiming;
	//用于本地判断我们现在是否在瞄准
	bool bLocalAiming = false;

	UFUNCTION()
		void OnRep_UnderAming();

	UPROPERTY(VisibleAnywhere)
		float BaseWalkSpeed;
	UPROPERTY(VisibleAnywhere)
		float AimWalkSpeed;

	//攻击
	UPROPERTY(Replicated,BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
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

	//当前装备在手上的武器备弹数
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
		int32 CarriedAmmo;
	UFUNCTION()
		void OnRep_CarriedAmmo();
	//存储武器和对应的备弹数
	TMap<EWeaponType, int32> CarriedAmmoMap;
	//最大备弹数
	UPROPERTY(EditAnywhere)
		int32 MaxAmmoAmount = 200;
	//步枪备弹数初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingARAmmo = 30;
	//火炮备弹数初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingRocketAmmo = 0;
	//手枪备弹数初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 StartingPistolAmmo = 15;
	//冲锋枪备弹初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 SubMachineGunAmmo = 55;
	//霰弹枪备弹初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 ShotGunAmmo = 10;
	//狙击枪备弹初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 SnipperAmmo = 10;
	//榴弹枪备弹初始值
	UPROPERTY(EditAnywhere, Category = "CarriedAmmo")
		int32 GrenadeLauncherAmmo = 10;
	//初始化Hash
	void InitializeCarriedAmmo();	

	//战斗状态是否换弹
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();

	/*默认武器*/
	UPROPERTY(EditAnywhere)
		TSubclassOf<AWeaponParent> DefaultWeaponClass;


	//对于装备武器的一些功能重写
protected:
	void ChangeEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackPackage(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeaponParent* WeaponToEquip);
	void ReloadWeaponAutomatic();
	//丢弃枪械
	void DropEquippedWeapon();
	//投掷时显示手雷
	void ShowAttachedGrenade(bool bShowGrenade);
	//交换武器
	void SwapWeapon();

public:	
	void EquipWeapon(AWeaponParent* WeaponToEquip);
	//装备主武器
	void EquipPrimaryWeapon(AWeaponParent* WeaponToEquip);
	//装备副武器
	void EquipSecondWeapon(AWeaponParent* WeaponToEquip);
	void NewEquipWeapon();
	//是否还有足够的子弹
	bool HaveAmmoCanFire();

	//重新装弹的播放控制
	void ReloadWeapon();

	//更新子弹和备弹数
	void UpdateAmmoAndCarriedAmmo();

	//专用于霰弹枪
	void ShotGunUpdateAmmoAndCarriedAmmo();

	//用于跳转到section，这样客户端和服务器都可以调用
	void JumpToShotGunEnd();

	//用于PickUp更新拾取后的备弹数
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	//丢枪时，枪内的子弹数
	UPROPERTY(VisibleAnywhere)
		int32 SavedAmmo;

	//生成默认武器
	void SpawnDefaultWeapon();
	//装备默认武器后更新对应的UI
	void UpdateHUDAmmo();

	//获取武器
	FORCEINLINE AWeaponParent* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE bool GetbAiming() const { return bUnderAiming; }
	FORCEINLINE int32 GetGrenades() const { return GrenadeAmount; }
	//能否交换武器
	bool CouldSwapWeapon();
	//本地控制是否开启FBRIK，因为我们在本地提前播放了换弹动画
	UPROPERTY(VisibleAnywhere)
		bool bLocallyReloading = false;

};
