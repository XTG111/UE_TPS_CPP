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

	//针对霰弹枪，添加一个该Box属于哪一个角色
	UPROPERTY()
		AXCharacter* Character;
};

//用于检测击中返回
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
	//是否击中
	UPROPERTY()
		bool bHitConfirmed;
	//是否爆头
	UPROPERTY()
		bool bHeadShot;
};

//用于检测霰弹枪击中返回
USTRUCT(BlueprintType)
struct FShotGunServerSideRewindResult
{
	GENERATED_BODY()
		//是否击中
	UPROPERTY()
		TMap<AXCharacter*, uint32> HeadShots;
	//是否爆头
	UPROPERTY()
		TMap<AXCharacter*, uint32> BodyShots;
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

	//在服务器上每Tick保存每一个FramePackeg
	void SaveFramePackage_Tick();

	//插值计算FramePackage的准确参数
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	//返回击中检测结果
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	//用于缓存被击中角色的HitBox
	void CacheBoxPosition(AXCharacter* HitCharacter, FFramePackage& OutFramePackage);

	//用于回退移动HitBox
	void MoveBoxes(AXCharacter* HitCharacter, const FFramePackage& Package);

	//将Box移动回来
	void ResetBoxes(AXCharacter* HitCharacter, const FFramePackage& Package);

	//关闭角色的骨骼网格体
	void EnableCharacterMeshCollision(AXCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable);
	
	//获取回退到HitTime的Box信息
	FFramePackage GetFrameToCheck(AXCharacter* HitCharacter, float HitTime);

	/*应用于霰弹枪的ServerRewide*/
	FShotGunServerSideRewindResult ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

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
	//计算HitTime的Box信息
	FServerSideRewindResult ServerSideRewind(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	/*应用于霰弹枪的ServerRewide*/
	FShotGunServerSideRewindResult ShotGunServerSideRewind(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);


	//用于计算是否真正的击中敌人之后的分数
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeaponParent* DamageCauser);

	//由于霰弹枪的攻击得分计算
	UFUNCTION(Server, Reliable)
		void ServerShotGunScoreRequest(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
};
