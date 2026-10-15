#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SentinelEnemy.generated.h"

class UHealthComponent;

/**
 * Guard enemy. Holds data and actions (health, attack, speeds).
 * The decision making lives in ASentinelAIController, so behaviour and body stay separate.
 */
UCLASS()
class SENTINEL_API ASentinelEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ASentinelEnemy();

	/** Waypoints this guard walks between. Assign per instance in the level. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "AI")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float PatrolSpeed = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ChaseSpeed = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackCooldown = 1.0f;

	void SetMoveSpeed(float Speed);
	bool CanAttack() const;
	void PerformAttack(AActor* Target);

	UHealthComponent* GetHealthComponent() const { return HealthComp; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	UFUNCTION()
	void HandleDeath();

private:
	float LastAttackTime = -1000.f;
};
