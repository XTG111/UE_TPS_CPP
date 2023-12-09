// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuSystemCharacter.generated.h"

UCLASS(config=Game)
class AMenuSystemCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AMenuSystemCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//����OnlineSubsystem
public:
	//���߻Ự����ӿڵ�ָ�룬����һ������ָ��
	//�����ֶ��巽ʽ
	/*
	* //��Ҫ��.cpp�ļ��а���#include "Interfaces/OnlineSessionInterface.h"����IOnlienSession��*/
	IOnlineSessionPtr OnlineSessionInterface;
	
	//�ڶ��������������ģ�嶨��
	//TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> OnlineSessionInterface;

	//SessionInterface�ص�ί��
protected:
	//��������Steam���봴��һ��Session
	UFUNCTION(BlueprintCallable)
		void CreateGameSession();

	//����һ���ͻ�������Session�İ���
	UFUNCTION(BlueprintCallable)
		void JoinGameSession();

	//�����ص�����
	//�Ƿ�ɹ�����ί��
	void OnCreateSessionComplete(FName SessionName,bool bWasSuccessful);
	//�Ƿ�Ѱ�ҵ�session
	void OnFindSessionComplete(bool bWasSuccessful);
	//Ѱ�ҵ�Session���һ���ص�����Join
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:

	//����ί��
	//�Ƿ񴴽��ɹ���ί��
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;

	//Ѱ�Ҵ��ڻỰ��ί��
	FOnFindSessionsCompleteDelegate FindSessionCompleteDelegate;

	//���ͻ���Ѱ�ҵ�Session,����ί���¼�����Join�¼�
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;

	//Ϊ�˻�ȡ��������session���������Ҫ�����Ϊȫ�ֱ���
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
};

