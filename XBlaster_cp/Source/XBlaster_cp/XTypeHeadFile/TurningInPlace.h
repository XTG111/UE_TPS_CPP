#pragma once

//����׼ƫ�ƴﵽһ���̶�ʵ��ת�򣬶���ö��������״̬
UENUM(BlueprintType)
enum class ETuringInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NoTurning UMETA(DisplayName = "Not Turning"),
	
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};