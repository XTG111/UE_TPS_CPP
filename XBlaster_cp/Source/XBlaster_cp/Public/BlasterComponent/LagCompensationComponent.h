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
};
