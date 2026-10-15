#include "SentinelEnemy.h"
#include "HealthComponent.h"
#include "SentinelAIController.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

ASentinelEnemy::ASentinelEnemy()
{
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	AIControllerClass = ASentinelAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
}

void ASentinelEnemy::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnDeath.AddDynamic(this, &ASentinelEnemy::HandleDeath);
}

void ASentinelEnemy::SetMoveSpeed(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

bool ASentinelEnemy::CanAttack() const
{
	return GetWorld() && (GetWorld()->GetTimeSeconds() - LastAttackTime) >= AttackCooldown;
}

void ASentinelEnemy::PerformAttack(AActor* Target)
{
	if (!Target || !CanAttack())
	{
		return;
	}

	LastAttackTime = GetWorld()->GetTimeSeconds();
	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void ASentinelEnemy::HandleDeath()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
		AIC->Destroy();
	}

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	SetLifeSpan(8.f);
}
