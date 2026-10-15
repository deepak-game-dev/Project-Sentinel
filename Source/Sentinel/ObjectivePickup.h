#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectivePickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class URotatingMovementComponent;

/** The data drive the player must collect before extraction. */
UCLASS()
class SENTINEL_API AObjectivePickup : public AActor
{
	GENERATED_BODY()

public:
	AObjectivePickup();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* Trigger;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPointLightComponent* Glow;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URotatingMovementComponent* Spinner;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
