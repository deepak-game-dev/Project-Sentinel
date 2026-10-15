#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SentinelHUD.generated.h"

/** Draws health, objective, hints and the mission result directly on the canvas (no UMG assets needed). */
UCLASS()
class SENTINEL_API ASentinelHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color);
};
