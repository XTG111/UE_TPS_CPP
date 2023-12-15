// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/XCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include <Kismet/GameplayStatics.h>

// Sets default values
AXCharacter::AXCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(GetMesh());
	SpringArmComp->TargetArmLength = 600.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp,USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

}

// Called when the game starts or when spawned
void AXCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AXCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AXCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AXCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AXCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AXCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &AXCharacter::Turn);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AXCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AXCharacter::StopJumping);
}

void AXCharacter::MoveForward(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	ForwardValue = value;
	FVector FowardVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::X);
	AddMovementInput(FowardVector, value);
}

void AXCharacter::MoveRight(float value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	RightValue = value;
	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	AddMovementInput(RightVector, value);
}

void AXCharacter::LookUp(float value)
{
	AddControllerYawInput(value);
}

void AXCharacter::Turn(float value)
{
	AddControllerPitchInput(value);
}

void AXCharacter::Jump()
{
	Super::Jump();
	bUnderJump = true;
}

void AXCharacter::StopJumping()
{
	Super::StopJumping();
	bUnderJump = false;
}



