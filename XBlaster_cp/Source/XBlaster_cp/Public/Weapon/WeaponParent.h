// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XBlaster_cp/XTypeHeadFile/WeaponTypes.h"
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

	//记录当前的枚举数量
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class XBLASTER_CP_API AWeaponParent : public AActor
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
	UPROPERTY(EditAnywhere,ReplicatedUsing = OnRep_Ammo)
		int32 Ammo = 30;
	UFUNCTION()
		void OnRep_Ammo();
	void SpendRound();
	void SetHUDAmmo();

	//丢掉武器
	void Drop();

	//武器类型控制
	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

	//换弹后更新子弹数
	void AddAmmo(int32 AmmoToAdd);

	//装备武器的音效
	UPROPERTY(EditAnywhere)
		class USoundCue* EquipSound;
	//没子弹开火音效
	UPROPERTY(EditAnywhere)
		USoundCue* DryFireSound;

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
};
