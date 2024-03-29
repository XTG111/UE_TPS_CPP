#pragma once
//当瞄准偏移达到一定程度实现转向，定义枚举来控制状态
UENUM(BlueprintType)
enum class ETuringInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NoTurning UMETA(DisplayName = "Not Turning"),
	
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};

//定义说明我们自己添加的Channel
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
//继续添加Channel
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2