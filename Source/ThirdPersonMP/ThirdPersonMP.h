// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(LogThirdPersonMP, Log, All);

// ============================================================================
// 调试信息宏配置
// ============================================================================
// 设置为 1 启用屏幕调试信息（GEngine方式，多窗口共享），设置为 0 禁用
#define ENABLE_SCREEN_DEBUG_MESSAGES 0

// 设置为 1 启用3D世界调试信息（DrawDebugString方式，每个窗口独立显示），设置为 0 禁用
#define ENABLE_WORLD_DEBUG_MESSAGES 1

// 屏幕调试信息宏（已禁用，多窗口PIE时所有窗口显示相同信息）
#if ENABLE_SCREEN_DEBUG_MESSAGES
	#define SCREEN_DEBUG_MESSAGE(Key, Time, Color, Message) \
		if (GEngine) { GEngine->AddOnScreenDebugMessage(Key, Time, Color, Message); }
#else
	#define SCREEN_DEBUG_MESSAGE(Key, Time, Color, Message)
#endif

// 3D世界调试信息宏（推荐用于多窗口PIE调试，每个窗口显示各自角色的信息）
// 在角色头顶显示调试文本，每个窗口独立渲染
#if ENABLE_WORLD_DEBUG_MESSAGES
	#include "DrawDebugHelpers.h"
	#define WORLD_DEBUG_MESSAGE(WorldContext, Location, Time, Color, Message) \
		if (WorldContext) { \
			DrawDebugString(WorldContext, Location, Message, nullptr, Color, Time, true, 1.2f); \
		}
#else
	#define WORLD_DEBUG_MESSAGE(WorldContext, Location, Time, Color, Message)
#endif