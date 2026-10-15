#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "SentinelAIController.generated.h"

class UAISenseConfig_Sight;
class ASentinelEnemy;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Patrol,
	Chase,
	Attack,
	Investigate
};

/**
 * Enemy brain: a finite state machine driven by AI Perception (sight) and NavMesh movement.
 *
 *  Patrol -> (player seen) -> Chase -> (in range) -> Attack
 *  Chase/Attack -> (player lost) -> Investigate last known position -> Patrol
 */
UCLASS()
class SENTINEL_API ASentinelAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASentinelAIController();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyState State = EEnemyState::Patrol;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	/** Seconds spent searching at the last known position before returning to patrol. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SearchDuration = 4.f;

	/** Seconds spent waiting at each patrol point. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float PatrolWaitTime = 1.5f;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void SetState(EEnemyState NewState);
	void TickPatrol(float Dt);
	void TickChase(float Dt);
	void TickAttack(float Dt);
	void TickInvestigate(float Dt);
	bool IsTargetAlive() const;
	void DrawDebugInfo() const;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	FVector LastKnownLocation = FVector::ZeroVector;
	int32 PatrolIndex = -1;
	float PatrolWaitTimer = 0.f;
	float SearchTimer = 0.f;
	float RepathTimer = 0.f;
};
