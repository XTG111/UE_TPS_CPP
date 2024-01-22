// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class XBLASTER_CP_API UReturnToMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//处理菜单
	void MenuSet();
	//销毁菜单
	void MenuTearDown();

protected:
	virtual bool Initialize() override;

	//设置绑定到子系统中的销毁委托
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);

	//角色离开游戏时的委托回调函数
	UFUNCTION()
		void OnPlayerLeftGame();

private:
	UPROPERTY()
		class APlayerController* PlayerController;
	UPROPERTY(meta = (BindWidget))
		class UButton* Return2MainMenuButton;
	//回调函数
	UFUNCTION()
		void Return2MainMenuButtonClicked();
	//会话连接
	UPROPERTY()
		class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;
};
