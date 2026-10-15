#include "ExtractionZone.h"
#include "SentinelGameMode.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AExtractionZone::AExtractionZone()
{
	PrimaryActorTick.bCanEverTick = true;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	Zone->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	Zone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = Zone;
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();
	Zone->OnComponentBeginOverlap.AddDynamic(this, &AExtractionZone::OnBeginOverlap);
}

void AExtractionZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if !UE_BUILD_SHIPPING
	// Visible outline so the player can find the zone during the demo.
	DrawDebugBox(GetWorld(), Zone->GetComponentLocation(), Zone->GetScaledBoxExtent(), FColor::Green, false, 0.f, 0, 6.f);
#endif
}

void AExtractionZone::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}

	if (ASentinelGameMode* GM = GetWorld()->GetAuthGameMode<ASentinelGameMode>())
	{
		GM->OnExtractionReached();
	}
}
