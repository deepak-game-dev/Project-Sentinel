#include "SentinelAIController.h"
#include "SentinelEnemy.h"
#include "HealthComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

namespace
{
	const TCHAR* StateToText(EEnemyState S)
	{
		switch (S)
		{
		case EEnemyState::Patrol:      return TEXT("PATROL");
		case EEnemyState::Chase:       return TEXT("CHASE");
		case EEnemyState::Attack:      return TEXT("ATTACK");
		case EEnemyState::Investigate: return TEXT("INVESTIGATE");
		}
		return TEXT("?");
	}
}

ASentinelAIController::ASentinelAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp")));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	GetPerceptionComponent()->ConfigureSense(*SightConfig);
	GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ASentinelAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ASentinelAIController::OnTargetPerceptionUpdated);
	}

	if (ASentinelEnemy* Enemy = Cast<ASentinelEnemy>(InPawn))
	{
		Enemy->SetMoveSpeed(Enemy->PatrolSpeed);
	}
}

void ASentinelAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// This prototype only cares about the player.
	if (!Actor || Actor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		TargetActor = Actor;
		LastKnownLocation = Actor->GetActorLocation();
		if (State != EEnemyState::Attack)
		{
			SetState(EEnemyState::Chase);
		}
	}
	else
	{
		// Lost sight: remember where we last saw the player and go check.
		TargetActor = nullptr;
		LastKnownLocation = Stimulus.StimulusLocation;
		SetState(EEnemyState::Investigate);
	}
}

void ASentinelAIController::SetState(EEnemyState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	RepathTimer = 0.f;
	SearchTimer = 0.f;
	PatrolWaitTimer = 0.f;

	ASentinelEnemy* Enemy = Cast<ASentinelEnemy>(GetPawn());
	if (!Enemy)
	{
		return;
	}

	switch (State)
	{
	case EEnemyState::Patrol:
		Enemy->SetMoveSpeed(Enemy->PatrolSpeed);
		StopMovement();
		break;
	case EEnemyState::Chase:
		Enemy->SetMoveSpeed(Enemy->ChaseSpeed);
		break;
	case EEnemyState::Attack:
		Enemy->SetMoveSpeed(Enemy->ChaseSpeed);
		StopMovement();
		break;
	case EEnemyState::Investigate:
		Enemy->SetMoveSpeed((Enemy->PatrolSpeed + Enemy->ChaseSpeed) * 0.5f);
		MoveToLocation(LastKnownLocation, 50.f);
		break;
	}
}

bool ASentinelAIController::IsTargetAlive() const
{
	if (!TargetActor)
	{
		return false;
	}
	const UHealthComponent* Health = TargetActor->FindComponentByClass<UHealthComponent>();
	return !Health || !Health->IsDead();
}

void ASentinelAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!GetPawn())
	{
		return;
	}

	switch (State)
	{
	case EEnemyState::Patrol:      TickPatrol(DeltaSeconds);      break;
	case EEnemyState::Chase:       TickChase(DeltaSeconds);       break;
	case EEnemyState::Attack:      TickAttack(DeltaSeconds);      break;
	case EEnemyState::Investigate: TickInvestigate(DeltaSeconds); break;
	}

	DrawDebugInfo();
}

void ASentinelAIController::TickPatrol(float Dt)
{
	ASentinelEnemy* Enemy = Cast<ASentinelEnemy>(GetPawn());
	if (!Enemy || Enemy->PatrolPoints.Num() == 0)
	{
		return;
	}

	// Idle means the last move finished (or none was started yet).
	if (GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		PatrolWaitTimer += Dt;
		if (PatrolWaitTimer >= PatrolWaitTime)
		{
			PatrolWaitTimer = 0.f;
			PatrolIndex = (PatrolIndex + 1) % Enemy->PatrolPoints.Num();
			if (AActor* Next = Enemy->PatrolPoints[PatrolIndex])
			{
				MoveToActor(Next, 50.f);
			}
		}
	}
}

void ASentinelAIController::TickChase(float Dt)
{
	ASentinelEnemy* Enemy = Cast<ASentinelEnemy>(GetPawn());
	if (!Enemy || !IsTargetAlive())
	{
		SetState(EEnemyState::Patrol);
		return;
	}

	const float Dist = FVector::Dist(Enemy->GetActorLocation(), TargetActor->GetActorLocation());
	if (Dist <= Enemy->AttackRange)
	{
		SetState(EEnemyState::Attack);
		return;
	}

	// Re-path a few times per second instead of every frame.
	RepathTimer -= Dt;
	if (RepathTimer <= 0.f)
	{
		MoveToActor(TargetActor, Enemy->AttackRange * 0.7f);
		RepathTimer = 0.25f;
	}
}

void ASentinelAIController::TickAttack(float Dt)
{
	ASentinelEnemy* Enemy = Cast<ASentinelEnemy>(GetPawn());
	if (!Enemy || !IsTargetAlive())
	{
		SetState(EEnemyState::Patrol);
		return;
	}

	const FVector ToTarget = TargetActor->GetActorLocation() - Enemy->GetActorLocation();

	// Hysteresis (1.3x) stops the enemy flickering between Chase and Attack at the edge of range.
	if (ToTarget.Size() > Enemy->AttackRange * 1.3f)
	{
		SetState(EEnemyState::Chase);
		return;
	}

	const FRotator Face(0.f, ToTarget.Rotation().Yaw, 0.f);
	Enemy->SetActorRotation(FMath::RInterpTo(Enemy->GetActorRotation(), Face, Dt, 8.f));
	Enemy->PerformAttack(TargetActor);
}

void ASentinelAIController::TickInvestigate(float Dt)
{
	// Idle means we arrived at the last known position (or could not path there).
	if (GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		SearchTimer += Dt;
		if (SearchTimer >= SearchDuration)
		{
			SetState(EEnemyState::Patrol);
		}
	}
}

void ASentinelAIController::DrawDebugInfo() const
{
#if !UE_BUILD_SHIPPING
	if (const APawn* P = GetPawn())
	{
		DrawDebugString(GetWorld(), P->GetActorLocation() + FVector(0.f, 0.f, 120.f),
		                FString(StateToText(State)), nullptr, FColor::White, 0.f, true);
	}
#endif
}
