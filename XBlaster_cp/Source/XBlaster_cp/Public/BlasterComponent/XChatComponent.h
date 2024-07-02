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

	//ִ������NetMulticast
	UFUNCTION(NetMulticast, UnReliable)
		void MultiExcuteCommand(const FString& CommandId, enum ECommandTypes CPT);

	//UI
	//���÷��ͷ���Title
	FString CommandsPrefix;
	//���ͷ�����
	FString SendName;

	//����UI����
	bool bToggleChat = false;
	void ToggleChatWindow();

	//��ʾ��Ҽ���ʱ����Ϣ
	bool bShowJoinMessage = true;
	FText JoinMessageText = FText::FromString(TEXT("Join Into The Server"));
	void JoinMessage();

public:
	//������Ϣ����������Ϣ��ɫ
	FLinearColor SetMessageTypeColor(enum EMessageTypes& MessageType);

	//�����������
	UFUNCTION(Server, UnReliable)
		void ServerSetName(const FString& Name);


	//Server ������Ϣ
	UFUNCTION(Server, UnReliable)
		void ServerEnterText(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, enum EChatTypes ChatType);
	void EnterText(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, EChatTypes ChatType);

	//Multi ����Message���㲥�����пͻ���
	UFUNCTION(NetMulticast, UnReliable)
		void MultiSendMessage(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, enum EChatTypes ChatType);
	void SendMessage(const FText& Message, EMessageTypes MessageType, const FString& PlayerName, const FLinearColor& color, enum EChatTypes ChatType);
};
