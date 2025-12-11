// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ThirdPersonMPHUD.generated.h"

/**
 * HUD class for displaying debug information in multiplayer games
 */
UCLASS()
class AThirdPersonMPHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Constructor */
	AThirdPersonMPHUD();

protected:
	/** Called every frame to draw HUD */
	virtual void DrawHUD() override;

private:
	/** Helper function to draw network debug info for a character */
	void DrawCharacterNetworkInfo(class AThirdPersonMPCharacter* Character, float& YPos);
};

