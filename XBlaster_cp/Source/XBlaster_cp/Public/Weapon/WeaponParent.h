// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
#include "XBlaster_cp/XTypeHeadFile/TeamState.h"
#include "Interfaces/FObjectInterface.h"
#include "WeaponParent.generated.h"
//作为武器父类被武器实例类继承

//设置武器状态的一个枚举值，用来控制我们的碰撞的开关
//就如同在UE中直接创建一个枚举变量的使用
UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Second UMETA(DisplayName = "Second"),

	//记录当前的枚举数量
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

//区分不同武器，随机弹道还是粒子，用于规范命中点
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_ShotGun UMETA(DispalyName = "Shot Gun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class XBLASTER_CP_API AWeaponParent : public AActor, public IFObjectInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponParent();

	//通过复制向客户端传递服务器的处理结果
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	//武器基本网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
		USkeletalMeshComponent* WeaponMesh;

	//球型碰撞检测组件，用于拾取操作等
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		class USphereComponent* SphereComp;
	

	//武器装备状态
	UPROPERTY(VisibleAnywhere, Category = "Weapon", ReplicatedUsing = OnRep_WeponState)
		EWeaponState WeaponState;

	//武器状态的Rep_Notify
	UFUNCTION()
		void OnRep_WeponState();

	//设置武器状态的函数
	UFUNCTION()
	void SetWeaponState(EWeaponState State);

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
		class UWidgetComponent* PickUpWidgetComp;

	UPROPERTY(EditAnywhere,Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;

	//是否显示UI
	void ShowPickUpWidget(bool bShowWidget);

	//用于开枪的控制,可以在子类覆盖
	virtual void Fire(const FVector& HitTarget);

	//抛壳类
	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABulletShellActor> BulletShellClass;

	//准星的绘制
	UPROPERTY(EditAnywhere,Category = Crosshairs)
		class UTexture2D* CrosshairCenter;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		class UTexture2D* CrosshairBottom;

	//Zoom FOV while aiming

	UPROPERTY(EditAnywhere)
		float ZoomFOV = 30.f;
	UPROPERTY(EditAnywhere)
		float ZoomInterpSpeed = 20.f;

	//自动开火
	UPROPERTY(EditAnywhere, Category = "Combat")
		float FireDelay = 0.1f;

	//控制是否时全自动
	UPROPERTY(EditAnywhere, Category = "Combat")
		bool bAutomatic = true;

	//角色类和控制器用于绘制HUD
	UPROPERTY()
		class AXCharacter* XCharacter;
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;

	//子弹数量控制
	//最大子弹数
	UPROPERTY(EditAnywhere)
		int32 MaxAmmo = 30;
	//当前子弹数
	UPROPERTY(EditAnywhere)
		int32 Ammo = 30;

	UFUNCTION(Client,Reliable)
		void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, Reliable)
		void ClientAddAmmo(int32 AmmoToAdd);

	//the number of unprocessed server request for ammo
	//Incremented in spendround decremented in clientupdateAmmo
	// 这个就是那个序号 不需要单独存储子弹数
	//相当于客户端记录发出了多少请求，即发射了多少次，然后次数*每次的子弹数即使最后的差值
	int32 Sequence = 0;

	void SpendRound();
	void SetHUDAmmo();

	//丢掉武器
	virtual void Drop();

	//武器类型控制
	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

	//团队类型
	UPROPERTY(EditAnywhere)
		ETeam TeamType;

	//换弹后更新子弹数
	void AddAmmo(int32 AmmoToAdd);

	//装备武器的音效
	UPROPERTY(EditAnywhere)
		class USoundCue* EquipSound;
	//没子弹开火音效
	UPROPERTY(EditAnywhere)
		USoundCue* DryFireSound;

	//判断现在是否满弹
	bool IsFull();

	//启用自定义深度
	void EnableCustomDepth(bool bEnable);

	//接口用于拾取武器
	void FPickObject_Implementation(APawn* InstigatorPawn);

	//是否可以销毁武器
	//只将默认武器设为true
	//其余保持false
	//在actor死亡时会销毁默认武器
	bool bDestroyWeapon = false;

	//规定每个武器的种类，用于决定是否向服务器传递命中点
	UPROPERTY(EditAnywhere)
		EFireType FireType;

	//计算得到散布的一点
	FVector TraceEndWithScatter(const FVector& HitTarget);
	/*
	Trace End with scatter
	*/
	//散布距离
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;
	//散布球体半径
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float SphereRadius = 75.f;
	//是否开启散布
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		bool bUseScatter = false;

	//武器伤害
	UPROPERTY(EditAnywhere, Category = "Damage")
		float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float HeadShotDamage = 40.f;

protected:
	//重叠事件响应回调函数,蓝图中的OnBeginOverlap节点
	UFUNCTION()
	virtual void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult
	);

	UFUNCTION()
		virtual void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex
		);

	//决定是否使用ServerRewide功能检测是否击中目标
	UPROPERTY(Replicated, EditAnywhere)
		bool bUseServerSideRewide = false;

	/*武器状态*/
	//决定每个武器状态有什么功能
	virtual void OnWeaponStateSet();
	virtual void HandleOnEquipped();
	virtual void HandleOnDropped();
	virtual void HandleOnSecond();

	/*High Ping*/
	UFUNCTION()
		void OnPingTooHigh(bool bPingTooHigh);
};
