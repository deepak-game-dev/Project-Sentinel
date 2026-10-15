#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SentinelGameMode.generated.h"

UENUM(BlueprintType)
enum class EMissionState : uint8
{
	InProgress,
	Succeeded,
	Failed
};

/**
 * Owns the mission rules: collect the data, reach extraction, don't die.
 * Pickups, the extraction zone and the player report events here; the HUD only reads state.
 */
UCLASS()
class SENTINEL_API ASentinelGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASentinelGameMode();

	UFUNCTION(BlueprintPure, Category = "Mission")
	EMissionState GetMissionState() const { return MissionState; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool HasData() const { return bHasData; }

	void OnDataCollected();
	void OnExtractionReached();
	void OnPlayerDied();
	void RestartMission();

	/** Temporary on-screen message (e.g. "Retrieve the data first"). */
	void ShowHint(const FString& Text, float Duration);
	bool GetActiveHint(FString& OutText) const;

private:
	EMissionState MissionState = EMissionState::InProgress;
	bool bHasData = false;
	FString HintText;
	float HintExpireTime = 0.f;
};
