// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
		void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("XTG111")));

protected:

	//��д��ʼ����������ʼ�������ͺ͹��캯������
	virtual bool Initialize() override;

	//������д��ʵ�ֵ�������ת�ؿ�ʱ��һЩ�߼�
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

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
	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("XTG111") };
	
};
