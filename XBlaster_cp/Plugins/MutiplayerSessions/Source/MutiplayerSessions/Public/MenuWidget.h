// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MUTIPLAYERSESSIONS_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//设置鼠标的可见性等
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:

	//重写初始化函数，初始化函数就和构造函数类似
	virtual bool Initialize() override;

	//利用重写，实现当我们跳转关卡时的一些逻辑
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	//自定义委托的回调函数，委托在UMultiPlayerSessionSubsystem
	//
	UFUNCTION()
		void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
		void OnStartSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

private:
	//按钮,利用meta指定变量链接到按钮的控件
	//变量名字必须与蓝图UI上面的名字一样
	UPROPERTY(meta = (BindWidget))
		class UButton* Button_Host;
	UPROPERTY(meta = (BindWidget))
		UButton* Button_Join;

	//点击之后的回调函数
	UFUNCTION()
		void Button_HostClicked();

	UFUNCTION()
		void Button_JoinClicked();

	//创建插件子系统的变量，用来使得按下按钮能够调用其中的事件
	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	//跳转后，恢复角色的控制，并删除UI
	void MenuTearDown();

	//一些参数，实现自定义输入
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{ 4 };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{ TEXT("FreeForAll") };

	//登录大厅
	FString PathToLobby{ TEXT("") };
	
};
