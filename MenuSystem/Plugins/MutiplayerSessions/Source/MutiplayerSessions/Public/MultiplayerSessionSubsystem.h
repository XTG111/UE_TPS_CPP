// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MUTIPLAYERSESSIONS_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionSubsystem();

	//����Session�ӿ���Ҫ�ĺ�������������Ե�������
	//����������Sessionʱ����
	//ͨ��������������Session�����ã�һ�����ܹ����ӵ���ң�һ���ǵȴ��ͻ�������ʱ��Ҫƥ��ļ�ֵ��ֵ
	void CreateSession(int32 NumPublicConnections, FString MatchType);

	//�ͻ���Ѱ��Sessionʱ���ã������Ѱ��
	void FindSessions(int32 MaxSearchResults);

	//�ͻ��˼���Sessionʱ���ô�����ȷ��Session�Ự
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	//���ٻỰ�Ϳ�ʼ�Ự
	void DestroySession();
	void StartSession();

protected:

	//�ص�����,���������ί�ж����
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	//Session�ӿ�
	IOnlineSessionPtr SessionInterface;

	//�ص�������Ӧ��ί��,�Լ�ί�ж�Ӧ��Handle
	//Handle����ά������ɾ����Ӧ��ί��
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

};
