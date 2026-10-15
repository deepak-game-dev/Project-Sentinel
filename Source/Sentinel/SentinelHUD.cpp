#include "SentinelHUD.h"
#include "SentinelGameMode.h"
#include "HealthComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

void ASentinelHUD::DrawCentered(const FString& Text, float Y, float Scale, const FLinearColor& Color)
{
	UFont* Font = GEngine->GetMediumFont();
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	DrawText(Text, Color, (Canvas->SizeX - W) * 0.5f, Y, Font, Scale);
}

void ASentinelHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !GEngine)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const ASentinelGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ASentinelGameMode>() : nullptr;
	const float Margin = 30.f;

	// Health bar
	float Percent = 0.f;
	if (const APawn* P = GetOwningPawn())
	{
		if (const UHealthComponent* Health = P->FindComponentByClass<UHealthComponent>())
		{
			Percent = Health->GetHealthPercent();
		}
	}
	const float BarW = 300.f, BarH = 22.f;
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), Margin, Margin, BarW + 4.f, BarH + 4.f);
	DrawRect(FLinearColor(0.8f, 0.15f, 0.15f, 1.f), Margin + 2.f, Margin + 2.f, BarW * Percent, BarH);
	DrawText(TEXT("HEALTH"), FLinearColor::White, Margin, Margin + BarH + 10.f, Font, 1.f);

	// Objective
	if (GM)
	{
		const FString Objective = GM->HasData()
			? TEXT("OBJECTIVE: Reach the extraction zone")
			: TEXT("OBJECTIVE: Retrieve the data");
		DrawText(Objective, FLinearColor(0.4f, 0.8f, 1.f, 1.f), Margin, Margin + BarH + 40.f, Font, 1.f);

		FString Hint;
		if (GM->GetActiveHint(Hint))
		{
			DrawCentered(Hint, Canvas->SizeY * 0.78f, 1.2f, FLinearColor::Yellow);
		}
	}

	// Controls reminder
	DrawText(TEXT("WASD move | Mouse look | F or Left Click attack | R restart"), FLinearColor(0.8f, 0.8f, 0.8f, 1.f),
	         Margin, Canvas->SizeY - 40.f, Font, 0.9f);

	// Mission result
	if (GM && GM->GetMissionState() != EMissionState::InProgress)
	{
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);
		const bool bWon = GM->GetMissionState() == EMissionState::Succeeded;
		DrawCentered(bWon ? TEXT("MISSION COMPLETE") : TEXT("MISSION FAILED"), Canvas->SizeY * 0.38f, 3.f,
		             bWon ? FLinearColor(0.3f, 1.f, 0.4f, 1.f) : FLinearColor(1.f, 0.3f, 0.3f, 1.f));
		DrawCentered(TEXT("Press R to restart"), Canvas->SizeY * 0.55f, 1.5f, FLinearColor::White);
	}
}
