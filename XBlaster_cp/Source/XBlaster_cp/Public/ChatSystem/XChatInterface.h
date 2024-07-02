// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../XTypeHeadFile/XChatEnum.h"
#include "XChatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UXChatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class XBLASTER_CP_API IXChatInterface
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void ChatMessage(const FText& Message, EMessageTypes Type, const FString& PlayerName, FLinearColor Color, EChatTypes ChatType);
};
