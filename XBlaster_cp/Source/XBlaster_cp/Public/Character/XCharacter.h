// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "XCharacter.generated.h"

UCLASS()
class XBLASTER_CP_API AXCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AXCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	//ÒÆ¶¯
	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);
	virtual void Jump() override;
	virtual void StopJumping() override;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float ForwardValue;
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		float RightValue;


private:
	UPROPERTY(VisibleAnywhere, Category= Camera)
		class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* CameraComp;

public:	
	UPROPERTY(BlueprintReadOnly, Category = MoveFunc)
		bool bUnderJump = false;


};
