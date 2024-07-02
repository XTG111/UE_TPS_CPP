// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChatSystem/XChatInterface.h"
#include "XChatComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XBLASTER_CP_API UXChatComponent : public UActorComponent, public IXChatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UXChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void ChatMessage_Implementation(const FText& Message, EMessageTypes Type, const FString& PlayerName, FLinearColor Color, EChatTypes ChatType);
	void SetSendName();

	//执行输入NetMulticast
	UFUNCTION(NetMulticast, UnReliable)
		void MultiExcuteCommand(const FString& CommandId, enum ECommandTypes CPT);

	//UI
	//设置发送方的Title
	FString CommandsPrefix;
	//发送方姓名
	FString SendName;

	//控制UI动画
	bool bToggleChat = false;
	void ToggleChatWindow();

	//显示玩家加入时的信息
	bool bShowJoinMessage = true;
	FText JoinMessageText = FText::FromString(TEXT("Join Into The Server"));
	void JoinMessage();

public:
	//根据消息类型设置消息颜色
	FLinearColor SetMessageTypeColor(enum EMessageTypes& MessageType);

	//设置玩家名字
	UFUNCTION(Server, UnReliable)
		void ServerSetName(const FString& Name);


	//Server 发送信息
	UFUNCTION(Server, UnReliable)
		void ServerEnterText(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, enum EChatTypes ChatType);
	void EnterText(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, EChatTypes ChatType);

	//Multi 传播Message，广播到所有客户端
	UFUNCTION(NetMulticast, UnReliable)
		void MultiSendMessage(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, enum EChatTypes ChatType);
	void SendMessage(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, enum EChatTypes ChatType);
};
