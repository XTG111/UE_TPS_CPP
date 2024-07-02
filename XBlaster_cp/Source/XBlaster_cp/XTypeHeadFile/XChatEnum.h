#pragma once
//Chat对象类型
UENUM(BlueprintType)
enum class EChatTypes :uint8
{
	ECT_All UMETA(DisplayName = "All"),
	ECT_Server UMETA(DisplayName = "Server"),
	ECT_Team UMETA(DisplayName = "Team"),
	ECT_Guild UMETA(DisplayName = "Guild"),
	ECT_Private UMETA(DisplayName = "Private"),
	ECT_DirectMessage UMETA(DisplayName = "DirectMessage"),

	//ECT_MAX UMETA(DisplayName = "DefaultMAX")
};

//Chat调试
UENUM(BlueprintType)
enum class ECommandTypes :uint8
{
	ECoT_SetName UMETA(DisplayName = "SetName"),
	ECoT_PrintHello UMETA(DisplayName = "PrintHello"),

	//ECoT_MAX UMETA(DisplayName = "DefaultMAX")
};

//Chat发送信息的类型
UENUM(BlueprintType)
enum class EMessageTypes :uint8
{
	EMT_NormalMessage UMETA(DisplayName = "NormalMessage"),
	EMT_Server UMETA(DisplayName = "Server"),
	EMT_Error UMETA(DisplayName = "Error"),
	EMT_Warning UMETA(DisplayName = "Warning"),
	EMT_Success UMETA(DisplayName = "Success"),
	EMT_DirectMessage UMETA(DisplayName = "DirectMessage"),

	//EMT_MAX UMETA(DisplayName = "DefaultMAX")
};