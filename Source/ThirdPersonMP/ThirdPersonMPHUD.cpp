// Copyright Epic Games, Inc. All Rights Reserved.

#include "ThirdPersonMPHUD.h"
#include "ThirdPersonMPCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"

AThirdPersonMPHUD::AThirdPersonMPHUD()
{
	// HUD默认启用
}

void AThirdPersonMPHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// 获取本地玩家控制器
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// ========== 在右上角显示 Ping 值 ==========
	if (PC->PlayerState)
	{
		// 获取 Ping 值（单位：毫秒）
		float PingMs = PC->PlayerState->GetPingInMilliseconds();

		// 根据 Ping 值选择颜色
		FColor PingColor;
		if (PingMs < 50.0f)
		{
			PingColor = FColor::Green;      // 优秀
		}
		else if (PingMs < 100.0f)
		{
			PingColor = FColor::Yellow;     // 良好
		}
		else if (PingMs < 200.0f)
		{
			PingColor = FColor::Orange;     // 一般
		}
		else
		{
			PingColor = FColor::Red;        // 较差
		}

		// 在右上角显示 Ping
		FString PingText = FString::Printf(TEXT("Ping: %.0f ms"), PingMs);

		// 计算文本宽度以实现右对齐
		float TextWidth, TextHeight;
		GetTextSize(PingText, TextWidth, TextHeight, nullptr, 1.0f);

		float XPos = Canvas->SizeX - TextWidth - 20.0f;  // 距离右边缘20像素
		float YPos = 20.0f;  // 距离顶部20像素

		DrawText(PingText, PingColor, XPos, YPos, nullptr, 1.0f);
	}

	// ========== 原有的调试信息 ==========
	// 设置起始Y位置
	float YPos = 50.0f;
	const float LineHeight = 20.0f;

	APawn* ControlledPawn = PC->GetPawn();
	AThirdPersonMPCharacter* LocalCharacter = Cast<AThirdPersonMPCharacter>(ControlledPawn);

	// 显示标题
	FString TitleText = TEXT("=== Network Debug Info ===");
	DrawText(TitleText, FColor::White, 10.0f, YPos, nullptr, 1.2f);
	YPos += LineHeight * 1.5f;

	// 显示本地控制的角色信息
	if (LocalCharacter)
	{
		FString LocalTitle = TEXT("--- Local Controlled Character ---");
		DrawText(LocalTitle, FColor::Yellow, 10.0f, YPos, nullptr, 1.0f);
		YPos += LineHeight;

		DrawCharacterNetworkInfo(LocalCharacter, YPos);
		YPos += LineHeight;
	}

	// 显示所有其他角色的信息
	FString OthersTitle = TEXT("--- All Characters in World ---");
	DrawText(OthersTitle, FColor::Cyan, 10.0f, YPos, nullptr, 1.0f);
	YPos += LineHeight;

	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<AThirdPersonMPCharacter> It(World); It; ++It)
		{
			AThirdPersonMPCharacter* Character = *It;
			if (Character)
			{
				DrawCharacterNetworkInfo(Character, YPos);
			}
		}
	}
}

void AThirdPersonMPHUD::DrawCharacterNetworkInfo(AThirdPersonMPCharacter* Character, float& YPos)
{
	if (!Character || !Canvas)
	{
		return;
	}

	const float LineHeight = 20.0f;
	const float IndentX = 30.0f;

	// 获取角色名称
	FString CharacterName = Character->GetName();

	// 获取IsLocallyControlled状态
	bool bIsLocallyControlled = Character->IsLocallyControlled();
	FString LocallyControlledStr = bIsLocallyControlled ? TEXT("TRUE") : TEXT("FALSE");

	// 获取LocalRole
	FString RoleStr;
	FColor RoleColor = FColor::White;

	switch (Character->GetLocalRole())
	{
		case ROLE_None:
			RoleStr = TEXT("ROLE_None");
			RoleColor = FColor::Red;
			break;
		case ROLE_SimulatedProxy:
			RoleStr = TEXT("ROLE_SimulatedProxy");
			RoleColor = FColor::Cyan;
			break;
		case ROLE_AutonomousProxy:
			RoleStr = TEXT("ROLE_AutonomousProxy");
			RoleColor = FColor::Yellow;
			break;
		case ROLE_Authority:
			RoleStr = TEXT("ROLE_Authority");
			RoleColor = FColor::Green;
			break;
		default:
			RoleStr = TEXT("ROLE_Unknown");
			RoleColor = FColor::Magenta;
			break;
	}

	// 获取RemoteRole
	FString RemoteRoleStr;
	switch (Character->GetRemoteRole())
	{
		case ROLE_None:
			RemoteRoleStr = TEXT("ROLE_None");
			break;
		case ROLE_SimulatedProxy:
			RemoteRoleStr = TEXT("ROLE_SimulatedProxy");
			break;
		case ROLE_AutonomousProxy:
			RemoteRoleStr = TEXT("ROLE_AutonomousProxy");
			break;
		case ROLE_Authority:
			RemoteRoleStr = TEXT("ROLE_Authority");
			break;
		default:
			RemoteRoleStr = TEXT("ROLE_Unknown");
			break;
	}

	// 组合显示文本
	FString InfoText = FString::Printf(TEXT("[%s] LocallyControlled: %s | LocalRole: %s | RemoteRole: %s"),
		*CharacterName, *LocallyControlledStr, *RoleStr, *RemoteRoleStr);

	// 绘制文本
	DrawText(InfoText, RoleColor, IndentX, YPos, nullptr, 0.9f);
	YPos += LineHeight;

	// 显示生命值
	FString HealthText = FString::Printf(TEXT("Health: %f"), Character->GetCurrentHealth());
	DrawText(HealthText, FColor::White, IndentX, YPos, nullptr, 0.9f);
	YPos += LineHeight;
}

