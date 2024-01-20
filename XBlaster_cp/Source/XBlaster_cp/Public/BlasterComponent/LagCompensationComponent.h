// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

//һ��BOX��Ҫ�洢 ��Ϣ
USTRUCT(BlueprintType)
struct FBoxInfomation
{
	GENERATED_BODY()

		UPROPERTY()
		FVector Location;
	UPROPERTY()
		FRotator Rotation;
	//���ӷ�Χ
	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct  FFramePackage
{
	GENERATED_BODY()

	//��Ҫ�洢����BOX��Χ�е���Ϣ
	UPROPERTY()
		float Time;

	//ʹ��Map ��ÿһ��������Ӧһ��BOX�������
	UPROPERTY()
		TMap<FName, FBoxInfomation> HitBoxInfoMap;

	//�������ǹ�����һ����Box������һ����ɫ
	UPROPERTY()
		AXCharacter* Character;
};

//���ڼ����з���
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
	//�Ƿ����
	UPROPERTY()
		bool bHitConfirmed;
	//�Ƿ�ͷ
	UPROPERTY()
		bool bHeadShot;
};

//���ڼ������ǹ���з���
USTRUCT(BlueprintType)
struct FShotGunServerSideRewindResult
{
	GENERATED_BODY()
		//�Ƿ����
	UPROPERTY()
		TMap<AXCharacter*, uint32> HeadShots;
	//�Ƿ�ͷ
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

	//����֡���ĺ���
	void SaveFramePackage(FFramePackage& Package);

	//�ڷ�������ÿTick����ÿһ��FramePackeg
	void SaveFramePackage_Tick();

	//��ֵ����FramePackage��׼ȷ����
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	//���ػ��м����
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);

	//���ڻ��汻���н�ɫ��HitBox
	void CacheBoxPosition(AXCharacter* HitCharacter, FFramePackage& OutFramePackage);

	//���ڻ����ƶ�HitBox
	void MoveBoxes(AXCharacter* HitCharacter, const FFramePackage& Package);

	//��Box�ƶ�����
	void ResetBoxes(AXCharacter* HitCharacter, const FFramePackage& Package);

	//�رս�ɫ�Ĺ���������
	void EnableCharacterMeshCollision(AXCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable);
	
	//��ȡ���˵�HitTime��Box��Ϣ
	FFramePackage GetFrameToCheck(AXCharacter* HitCharacter, float HitTime);

	/*Ӧ��������ǹ��ServerRewide*/
	FShotGunServerSideRewindResult ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:

	//��ɫ
	UPROPERTY()
		AXCharacter* XCharacter;
	//��ɫ������
	UPROPERTY()
		class AXBlasterPlayerController* XCharacterController;

	//FrameHistory
	TDoubleLinkedList<FFramePackage> FrameHistory;

	//�֡�洢ʱ��
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:
	//debug ��ʾFramePackage
	void ShowFramePackage(const FFramePackage& Package, const FColor Color);
	//����HitTime��Box��Ϣ
	FServerSideRewindResult ServerSideRewind(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	
	/*Ӧ��������ǹ��ServerRewide*/
	FShotGunServerSideRewindResult ShotGunServerSideRewind(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);


	//���ڼ����Ƿ������Ļ��е���֮��ķ���
	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(AXCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, class AWeaponParent* DamageCauser);

	//��������ǹ�Ĺ����÷ּ���
	UFUNCTION(Server, Reliable)
		void ServerShotGunScoreRequest(const TArray<AXCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
};
