// Fill out your copyright notice in the Description page of Project Settings.


#include "XPlayerState/XBlasterPlayerState.h"
#include "PlayerController/XBlasterPlayerController.h"
#include "Character/XCharacter.h"

void AXBlasterPlayerState::AddToScore(float ScoreAmount)
{
	//服务器处理得分的增加
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
	//PlayerState可以直接获取当前Pawn但不能获取Controller
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


