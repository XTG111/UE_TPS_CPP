// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponParent.generated.h"
//��Ϊ�������౻����ʵ����̳�

//��������״̬��һ��ö��ֵ�������������ǵ���ײ�Ŀ���
//����ͬ��UE��ֱ�Ӵ���һ��ö�ٱ�����ʹ��
UENUM(BlueprintType)
enum class EWeaponState :uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	//��¼��ǰ��ö������
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};


UCLASS()
class XBLASTER_CP_API AWeaponParent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponParent();

	//ͨ��������ͻ��˴��ݷ������Ĵ�����
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	//��������������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
		USkeletalMeshComponent* WeaponMesh;

	//������ײ������������ʰȡ������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		class USphereComponent* SphereComp;
	

	//����װ��״̬
	UPROPERTY(VisibleAnywhere, Category = "Weapon", ReplicatedUsing = OnRep_WeponState)
		EWeaponState WeaponState;

	//����״̬��Rep_Notify
	UFUNCTION()
		void OnRep_WeponState();

	//��������״̬�ĺ���
	UFUNCTION()
	void SetWeaponState(EWeaponState State);

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
		class UWidgetComponent* PickUpWidgetComp;

	UPROPERTY(EditAnywhere,Category = "Weapon Properties")
		class UAnimationAsset* FireAnimation;

	//�Ƿ���ʾUI
	void ShowPickUpWidget(bool bShowWidget);

	//���ڿ�ǹ�Ŀ���,���������า��
	virtual void Fire(const FVector& HitTarget);

	//�׿���
	UPROPERTY(EditAnywhere)
		TSubclassOf<class ABulletShellActor> BulletShellClass;

protected:
	//�ص��¼���Ӧ�ص�����,��ͼ�е�OnBeginOverlap�ڵ�
	UFUNCTION()
	virtual void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult
	);

	UFUNCTION()
		virtual void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex
		);
};
