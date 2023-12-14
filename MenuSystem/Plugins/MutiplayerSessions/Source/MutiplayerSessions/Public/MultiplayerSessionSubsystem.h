// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

//
//绑定自己创建的动态多播用于响应菜单UI界面的响应回调
//委托名字，参数类型，参数形参
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiPlayerOnCreateSessionComplete, bool, bWasSuccessful);

//动态多播的返回类型必须是UClass或者UStruct,并且与蓝图兼容

//寻找委托，返回一个数组结果，FOnlineSessionSearchResult不是一个UClass，所以在蓝图中也无法调用
//如果想在蓝图中调用，那么可以自己创建一个继承的UClass，然后采用动态多播
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiPlayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

//加入会话的委托传入加入的结果
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

	//
	//用于响应菜单UI的自定义委托
	//MultiPlayerOnCreateSessionComplete创建会话时的委托类似CreateSessionCompleteDelegate
	FMultiPlayerOnCreateSessionComplete MultiPlayerOnCreateSessionComplete;
	FMultiPlayerOnFindSessionComplete MultiPlayerOnFindSessionComplete;
	FMultiPlayerOnJoinSessionComplete MultiPlayerOnJoinSessionComplete;
	FMultiPlayerOnDestroySessionComplete MultiPlayerOnDestroySessionComplete;
	FMultiPlayerOnStartSessionComplete MultiPlayerOnStartSessionComplete;

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
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

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

	//用来判断是否还在删除，还是删除完成
	bool bCreateSessionOnDestroy{ false };

	//存储一些创建的信息，创建的连接数量和匹配的键值对
	int32 LastNumPublicConnections;
	FString LastMatchType;

};
