#pragma once

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType :uint8
{
	//粒子类型武器
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),
	//射线检测类型武器
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubMachineGun UMETA(DisplayName = "SubMachineGun"),
	EWT_ShotGun UMETA(DisplayName = "ShotGun"),
	EWT_Snipper UMETA(DisplayName = "Snipper"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};