// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	//是否显示UI
	void ShowPickUpWidget(bool bShowWidget);

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
