#include "SentinelCharacter.h"
#include "HealthComponent.h"
#include "SentinelGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASentinelCharacter::ASentinelCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ASentinelCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnDeath.AddDynamic(this, &ASentinelCharacter::HandleDeath);
}

void ASentinelCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASentinelCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASentinelCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASentinelCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASentinelCharacter::LookUpAtRate);

	// Bound directly to keys so no Project Settings change is needed.
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASentinelCharacter::Attack);
	PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &ASentinelCharacter::Attack);
	PlayerInputComponent->BindKey(EKeys::R, IE_Pressed, this, &ASentinelCharacter::RestartMission);
}

void ASentinelCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASentinelCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASentinelCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASentinelCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ASentinelCharacter::Attack()
{
	UWorld* World = GetWorld();
	if (!World || HealthComp->IsDead())
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastAttackTime < AttackCooldown)
	{
		return;
	}
	LastAttackTime = Now;

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerAttack), false, this);
	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn,
	                                              FCollisionShape::MakeSphere(AttackRadius), Params);

#if !UE_BUILD_SHIPPING
	DrawDebugSphere(World, End, AttackRadius, 12, bHit ? FColor::Red : FColor::Green, false, 0.4f);
#endif

	if (bHit && Hit.GetActor())
	{
		UGameplayStatics::ApplyDamage(Hit.GetActor(), AttackDamage, GetController(), this, UDamageType::StaticClass());
	}
}

void ASentinelCharacter::HandleDeath()
{
	GetCharacterMovement()->DisableMovement();

	if (ASentinelGameMode* GM = GetWorld()->GetAuthGameMode<ASentinelGameMode>())
	{
		GM->OnPlayerDied();
	}
	UE_LOG(LogTemp, Warning, TEXT("Player died"));
}

void ASentinelCharacter::RestartMission()
{
	if (ASentinelGameMode* GM = GetWorld()->GetAuthGameMode<ASentinelGameMode>())
	{
		GM->RestartMission();
	}
}
