#pragma once
UENUM(BlueprintType)
enum class EMatchType : uint8
{
	EMT_FreeForAll UMETA(DisplayName = "FreeForAll"),
	EMT_TeamsMatch UMETA(DisplayName = "TeamsMatch"),
	EMT_CTFMatch UMETA(DisplayName = "CTFMatch"),

	ET_MAX UMETA(DisplayName = "DefaultMax"),
};