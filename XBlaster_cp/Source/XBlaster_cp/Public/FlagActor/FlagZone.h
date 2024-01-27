// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "XBlaster_cp/XTypeHeadFile/TeamState.h"
#include "FlagZone.generated.h"

UCLASS()
class XBLASTER_CP_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlagZone();

	//��ǰ��������Ҫ���ܵ�TeamType
	UPROPERTY(EditAnywhere)
		ETeam TeamType;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//�ص��¼���Ӧ�ص�����,��ͼ�е�OnBeginOverlap�ڵ�
	UFUNCTION()
		virtual void OnSphereBeginOverlap(
			UPrimitiveComponent* OverlappedComponent, 
			AActor* OtherActor, 
			UPrimitiveComponent* OtherComponent, 
			int32 OtherBodyIndex, 
			bool bFromSweep, 
			const FHitResult& SweepResult
		);


private:	
	//��Ӧ����
	UPROPERTY(EditAnywhere)
		class USphereComponent* ZoneSphere;
};
