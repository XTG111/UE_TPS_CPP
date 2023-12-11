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

	//构建Session接口需要的函数，其他类可以调用他们
	//服务器创建Session时调用
	//通过传入参数方便对Session的设置，一个是能够连接的玩家，一个是等待客户端连接时需要匹配的键值对值
	void CreateSession(int32 NumPublicConnections, FString MatchType);

	//客户端寻找Session时调用，最大搜寻数
	void FindSessions(int32 MaxSearchResults);

	//客户端加入Session时调用传入正确的Session会话
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	//销毁会话和开始会话
	void DestroySession();
	void StartSession();

protected:

	//回调函数,其参数是由委托定义的
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	//Session接口
	IOnlineSessionPtr SessionInterface;

	//回调函数对应的委托,以及委托对应的Handle
	//Handle用来维护或者删除对应的委托
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
