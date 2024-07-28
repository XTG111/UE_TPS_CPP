// Fill out your copyright notice in the Description page of Project Settings.IInteractWithCrosshairInterface

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapon/WeaponParent.h"
#include "BlasterComponent/CombatComponent.h"
#include "BlasterComponent/XPropertyComponent.h"
#include "XBlaster_cp/XTypeHeadFile/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "XBlaster_cp/XTypeHeadFile/CombatState.h"
#include "XBlaster_cp/XTypeHeadFile/TeamState.h"
#include "XCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);
/*退出游戏的委托*/
UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AXCharacter();

	//通过复制向客户端传递服务器的处理结果
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//利用PostInitialize初始化组件中的变量
	virtual void PostInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//开枪动画
	void PlayFireMontage(bool bAiming);
	//被击中动画
	void PlayHitReactMontage();
	//死亡动画
	void PlayElimMontage();
	//换弹动画
	void PlayReloadMontage();
	//投掷动画
	void PlayGrenadeMontage();
	//交换武器的动画
	void PlaySwapMontage();

	//控制模拟机器上的转向动画，repnotify;
	virtual void OnRep_ReplicatedMovement() override;

	//更新血条
	UFUNCTION()
		void UpdateHUDHealth();
	UFUNCTION()
		void UpdateHUDShield();
	UFUNCTION()
		void ReceivedDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);


	//仅在服务器上调用死亡响应
	void Elim(bool bPlayerLeftGame);

	//响应死亡
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim(bool bPlayerLeftGame);

	//初始化PlayerState,用来更新对应的HUD,需要Tick检测，因为游戏的第一帧不会初始化PlayerState所以无法用BeginPlay进行初始化；
	//该函数将用于初始化任何无法在第一帧初始化的类
	void PollInit();

	//狙击枪开镜动画
	UFUNCTION(BlueprintImplementableEvent)
		void ShowSnipperScope(bool bShowScope);

	//根据队伍修改颜色
	void SetColorByTeam(ETeam team);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//移动
	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);
	void CrouchMode();
	void RelaxToAimMode();
	void AimToRelaxMode();
	virtual void Jump() override;
	virtual void StopJumping() override;
	void ReloadWeapon();
	void Grenade();
	void DropWeapon();
	void SwapWeapon();
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float ForwardValue;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float RightValue;

	//装备武器
	void EquipWeapon();

	//瞄准偏移AO_Yaw AO_Pitch
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	UPROPERTY(BlueprintReadOnly)
		float AO_Yaw;
	float AO_Pitch;

	//模拟代理转向
	void SimProxiesturn();

	//转向功能集合
	void RotateInPlace(float DeltaTime);


	FRotator StartingAimRotation;

	//瞄准偏移下的状态控制
	UPROPERTY(Replicated)
		ETuringInPlace TurningInPlace;

	//用于设置转动时根骨骼的转动 利用此值进行插值求得转动时的AO_Yaw
	float InterpAOYaw;
	////获取Yaw的角度变换，来修改状态
	UFUNCTION()
		void TurnInPlace(float DeltaTime);

	//攻击
	void Fireing();
	void ReFired();

	//蒙太奇动画
	UPROPERTY(EditAnywhere, Category = Combat)
		class UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ElimMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* GrenadeMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
		UAnimMontage* SwapMontage;

	void DroporDestroyWeapon(AWeaponParent* Weapon);

	//控制玩家出生点的设置,需要先确定我们的玩家团队在此之前设置好了
	void SetSpawnPoint();

	void OnPlayerStateInitialized();

public:
	//角色显示文字空间
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widget, meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverHeadWidget;
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* CameraComp;



	//向客户端传递重叠的武器，以显示UI界面
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponParent* OverlappingWeapon;
	//利用Rep Notify获取OverlappingWeapon的状态改变，从而调用显示UI的函数
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponParent* LastWeapon);

	//战斗组件设置
	UPROPERTY(VisibleAnywhere, Category = Combat, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UCombatComponent* CombatComp;

	//人物属性组件
	UPROPERTY(VisibleAnywhere, Category = CharacterPp)
		class UXPropertyComponent* PropertyComp;

	//Frame History Box组件
	UPROPERTY(VisibleAnywhere, Category = CharacterLag)
		class ULagCompensationComponent* LagCompensationComp;

	//RPC客户端调用，服务器执行，约定在函数名前加上Server
	UFUNCTION(Server, Reliable)
		void ServerEquipWeapon();
	UFUNCTION(Server, Reliable)
		void ServerSwapWeapon();

	//隐藏角色
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
		float CameraThreshold = 100.0f;

	//是否能够旋转根骨骼，网络传播根骨骼不是每一个tick都更新
	bool bRotateRootBone;
	//控制判断，用于模拟播放替代Yaw的转向动画
	float TurnThreshold = 0.5f;
	//记录模拟机器上上一帧和这一帧的旋转差值
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	//记录差值
	float ProxyYaw;
	//记录上次运动之后的Actor位置
	float TimeSinceLastMovementReplication;

	//整理求速度函数，在模拟代码中，如果速度大于0，要设置不能够转向
	float CalculateVelocity();

	//角色控制器
	UPROPERTY()
		class AXBlasterPlayerController* XBlasterPlayerController;

	//是否死亡
	bool bElimmed = false;

	//重生时间控制
	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.0f;
	void ElimTimerFinished();

	//死亡溶解效果
	// 

	//动态材质用于在代码中操作变量 change at Runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	//静态材质用于在蓝图中设置，是角色本身的材质实例 set on the BP,use for set the dynamic Material
	UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;
	//时间轴
	// 
	// 
	UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;
	//相当于蓝图中的Track
	FOnTimelineFloat DissolveTrack;

	//时间轴曲线
	UPROPERTY(EditAnywhere)
		UCurveFloat* DissolveCurve;

	//获取曲线上的值
	UFUNCTION()
		void UpdateDissolveMaterial(float Dissolve);
	//开始溶解
	void StartDissolve();


	/*
	* Team Colors
	*/
	//没有Team时的颜色
	UPROPERTY(EditAnywhere, Category = NoTeamColor)
		UMaterialInstance* OriginalMatInst;

	UPROPERTY(EditAnywhere, Category = TeamColors_Elim)
		UMaterialInstance* RedDissolveMatInst;
	UPROPERTY(EditAnywhere, Category = TeamColors_Elim)
		UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = TeamColors_Live)
		UMaterialInstance* RedMatInst;
	UPROPERTY(EditAnywhere, Category = TeamColors_Live)
		UMaterialInstance* BlueMatInst;

	//ElimBot
	UPROPERTY(EditAnywhere, Category = ElimBot)
		UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = ElimBot)
		UParticleSystemComponent* ElimBotComp;
	UPROPERTY(EditAnywhere, Category = ElimBot)
		class USoundCue* ElimBotSound;
	//ElimBot在客户端的消失，利用Destroyed()
	virtual void Destroyed() override;

	UPROPERTY()
		class AXBlasterGameMode* XBlasterGameMode;

	//为角色初始化PlayerState
	UPROPERTY()
		class AXBlasterPlayerState* XBlasterPlayerState;

	//Grenade component
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachGrenade;

	/*生成皇冠在领先的Actor
	*/
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* CrownSystem;
	UPROPERTY()
		class UNiagaraComponent* CrownComp;

	/*/*
	* 退出游戏的处理
	*/
	bool bLeftGame = false;

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = MoveFunc)
		bool bUnderJump = false;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerName)
		FString PlayerName = FString(TEXT("Player"));
	UFUNCTION()
		void OnRep_PlayerName();
	void SetPlayerName();

	UFUNCTION(Server, Reliable)
		void ServerSetLocalName(const FString& name, AXCharacter* nowactor);
	UFUNCTION(NetMulticast, Reliable)
		void MultiSetPlayerName(const TArray<FString>& actornames, const TArray<AActor*>& actorlist);



	//将重叠的变量获取到，然后传给客户端
	UFUNCTION()
		void SetOverlappingWeapon(AWeaponParent* Weapon);

	//设置是否装备，动画源码中通过调用这个函数，来设置是否装备了武器
	UFUNCTION()
		bool GetIsEquippedWeapon();

	//获取是否瞄准
	UFUNCTION()
		bool GetIsAiming();

	//给动画蓝图传入瞄准偏移的值
	UFUNCTION()
		float GetAOYawToAnim() const;
	UFUNCTION()
		float GetAOPitchToAnim() const;

	//给动画蓝图传入当前装备的武器
	UFUNCTION()
		AWeaponParent* GetEquippedWeapon();

	//设置获取状态值到动画蓝图
	ETuringInPlace GetTurninigInPlace() const;

	FVector GetHitTarget() const;

	//获取相机，传递给战斗组件，用于调整视角
	UFUNCTION()
		UCameraComponent* GetFollowCamera() const;

	//设置是否能够传递根骨骼
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE UCombatComponent* GetCombatComp() const { return CombatComp; }

	//获取玩家属性组件
	UXPropertyComponent* GetPropertyComp();
	//获取玩家控制器
	AXBlasterPlayerController* GetXBlasterPlayerCtr();
	//获取玩家此时是否在装弹，从而禁止动画中的IK
	ECombatState GetCombateState() const;

	FORCEINLINE UAnimMontage* GetRelodMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetGrenadeComp() const { return AttachGrenade; }
	FORCEINLINE bool GetbElimed() const { return bElimmed; }
	FORCEINLINE UXPropertyComponent* GetPropertyComp() const { return PropertyComp; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComp() const { return LagCompensationComp; }
	bool IsHoldingTheFlag() const;
	bool IsLocallyReloading();

	bool bFinishedSwap = false;

	/*
	* 角色退出游戏
	*/
	//通知服务器离开游戏
	UFUNCTION(Server, Reliable)
		void ServerLeaveGame();
	FOnLeftGame OnLeftGame;

	//显示领先者的皇冠到所有客户端
	//使用两个MultiRPC是为了避免网络带宽，否则我们需要传入参数来控制是否显示
	UFUNCTION(NetMulticast, Reliable)
		void MulticastGainerTheLead();
	UFUNCTION(NetMulticast, Reliable)
		void MulticastLostTheLead();

	ETeam GetTeam();

public:
	//控制哪些操作将被禁用
	UPROPERTY(Replicated)
		bool bDisableGamePlay = false;

	/*Mesh 包围盒BOX 用于充当服务器回退检测是否击中时的命中框*/
	UPROPERTY(EditAnywhere)
		class UBoxComponent* headbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvisbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02box;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03box;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* backpackbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* blanketbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_rbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_lbox;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_rbox;

	//存储上述Box
	UPROPERTY()
		TMap<FName, UBoxComponent*> HitBoxCompMap;

};
