// Fill out your copyright notice in the Description page of Project Settings.


#include "XPlayerState/XBlasterPlayerState.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Character/XCharacter.h"
#include "Net/UnrealNetwork.h"

void AXBlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AXBlasterPlayerState, Defeats);
}

void AXBlasterPlayerState::AddToScore(float ScoreAmount)
{
	//����������÷ֵ�����
	SetScore(GetScore() + ScoreAmount);
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetPawn()) : XCharacter;
	if (XCharacter)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->SetHUDScore(GetScore());
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Score: %d"), GetScore());
}

void AXBlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	//PlayerState����ֱ�ӻ�ȡ��ǰPawn�����ܻ�ȡController
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetPawn()) : XCharacter;
	if (XCharacter)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->SetHUDScore(GetScore());
		}
	}
}

void AXBlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	//��������������������
	Defeats += DefeatsAmount;
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetPawn()) : XCharacter;
	if (XCharacter)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void AXBlasterPlayerState::OnRep_Defeats()
{
	XCharacter = XCharacter == nullptr ? Cast<AXCharacter>(GetPawn()) : XCharacter;
	if (XCharacter)
	{
		XBlasterPlayerController = XBlasterPlayerController == nullptr ? Cast<AXBlasterPlayerController>(XCharacter->Controller) : XBlasterPlayerController;
		if (XBlasterPlayerController)
		{
			XBlasterPlayerController->SetHUDDefeats(Defeats);
		}
	}
}


