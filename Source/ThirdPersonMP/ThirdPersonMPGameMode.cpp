// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonMPGameMode.h"
#include "ThirdPersonMPHUD.h"

AThirdPersonMPGameMode::AThirdPersonMPGameMode()
{
	// 设置HUD类用于显示调试信息
	HUDClass = AThirdPersonMPHUD::StaticClass();
}
