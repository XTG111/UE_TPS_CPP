#pragma once

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_MAX UMETA(DisplayName = "DefaultMax")
};