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

	//�������Ŀɼ��Ե�
	UFUNCTION(BlueprintCallable)
		void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:

	//��д��ʼ����������ʼ�������ͺ͹��캯������
	virtual bool Initialize() override;

	//������д��ʵ�ֵ�������ת�ؿ�ʱ��һЩ�߼�
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	//�Զ���ί�еĻص�������ί����UMultiPlayerSessionSubsystem
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
	//��ť,����metaָ���������ӵ���ť�Ŀؼ�
	//�������ֱ�������ͼUI���������һ��
	UPROPERTY(meta = (BindWidget))
		class UButton* Button_Host;
	UPROPERTY(meta = (BindWidget))
		UButton* Button_Join;

	//���֮��Ļص�����
	UFUNCTION()
		void Button_HostClicked();

	UFUNCTION()
		void Button_JoinClicked();

	//���������ϵͳ�ı���������ʹ�ð��°�ť�ܹ��������е��¼�
	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	//��ת�󣬻ָ���ɫ�Ŀ��ƣ���ɾ��UI
	void MenuTearDown();

	//һЩ������ʵ���Զ�������
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{ 4 };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{ TEXT("FreeForAll") };

	//��¼����
	FString PathToLobby{ TEXT("") };
	
};
