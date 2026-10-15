#include "SentinelGameMode.h"
#include "SentinelHUD.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ASentinelGameMode::ASentinelGameMode()
{
	// Use the Third Person template's character Blueprint as the player.
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	HUDClass = ASentinelHUD::StaticClass();
}

void ASentinelGameMode::OnDataCollected()
{
	if (MissionState != EMissionState::InProgress)
	{
		return;
	}
	bHasData = true;
	ShowHint(TEXT("Data retrieved. Reach the extraction zone."), 4.f);
}

void ASentinelGameMode::OnExtractionReached()
{
	if (MissionState != EMissionState::InProgress)
	{
		return;
	}

	if (bHasData)
	{
		MissionState = EMissionState::Succeeded;
		UE_LOG(LogTemp, Log, TEXT("Mission succeeded"));
	}
	else
	{
		ShowHint(TEXT("Retrieve the data before extracting."), 3.f);
	}
}

void ASentinelGameMode::OnPlayerDied()
{
	if (MissionState == EMissionState::InProgress)
	{
		MissionState = EMissionState::Failed;
		UE_LOG(LogTemp, Log, TEXT("Mission failed"));
	}
}

void ASentinelGameMode::RestartMission()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

void ASentinelGameMode::ShowHint(const FString& Text, float Duration)
{
	HintText = Text;
	HintExpireTime = GetWorld() ? GetWorld()->GetTimeSeconds() + Duration : 0.f;
}

bool ASentinelGameMode::GetActiveHint(FString& OutText) const
{
	if (GetWorld() && GetWorld()->GetTimeSeconds() < HintExpireTime)
	{
		OutText = HintText;
		return true;
	}
	return false;
}
