// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//一个BOX需要存储 信息
USTRUCT(BlueprintType)
struct FBoxInfomation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;
	UPROPERTY()
		FRotator Rotation;
	//盒子范围
	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct  FFramePackage
{
	GENERATED_BODY()

		//需要存储所有BOX包围盒的信息
	UPROPERTY()
		float Time;

	//使用Map 对每一个骨骼对应一个BOX方便查找
	UPROPERTY()
		TMap<FName, FBoxInfomation> HitBoxInfoMap;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XBLASTER_CP_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class AXCharacter;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//保存帧包的函数
	void SaveFramePackage(FFramePackage& Package);

private:

	//角色
	UPROPERTY()
		AXCharacter* XCharacter;
	//角色控制器
	UPROPERTY()
		class AXBlasterPlayerController* XCharacterController;

	//FrameHistory
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//最长帧存储时间
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:
	//debug 显示FramePackage
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);
};
