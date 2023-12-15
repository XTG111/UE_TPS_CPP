// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

//
//���Լ������Ķ�̬�ಥ������Ӧ�˵�UI�������Ӧ�ص�
//ί�����֣��������ͣ������β�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnCreateSessionComplete, bool, bWasSuccessful);

//��̬�ಥ�ķ������ͱ�����UClass����UStruct,��������ͼ����

//Ѱ��ί�У�����һ����������FOnlineSessionSearchResult����һ��UClass����������ͼ��Ҳ�޷�����
//���������ͼ�е��ã���ô�����Լ�����һ���̳е�UClass��Ȼ����ö�̬�ಥ
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiPlayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

//����Ự��ί�д������Ľ��
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnStartSessionComplete, bool, bWasSuccessful);

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

	//
	//������Ӧ�˵�UI���Զ���ί��
	//MultiPlayerOnCreateSessionComplete�����Ựʱ��ί������CreateSessionCompleteDelegate
	FMultiPlayerOnCreateSessionComplete MultiPlayerOnCreateSessionComplete;
	FMultiPlayerOnFindSessionComplete MultiPlayerOnFindSessionComplete;
	FMultiPlayerOnJoinSessionComplete MultiPlayerOnJoinSessionComplete;
	FMultiPlayerOnDestroySessionComplete MultiPlayerOnDestroySessionComplete;
	FMultiPlayerOnStartSessionComplete MultiPlayerOnStartSessionComplete;

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
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

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

	//�����ж��Ƿ���ɾ��������ɾ�����
	bool bCreateSessionOnDestroy{ false };

	//�洢һЩ��������Ϣ������������������ƥ��ļ�ֵ��
	int32 LastNumPublicConnections;
	FString LastMatchType;

};
