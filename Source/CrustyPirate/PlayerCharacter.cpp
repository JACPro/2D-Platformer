#include "PlayerCharacter.h"

#include "Enemy.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter() 
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	AttackCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollisionBox"));
	AttackCollisionBox->SetupAttachment(RootComponent);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	OnAttackOverrideEndDelegate.BindUObject(this, &APlayerCharacter::OnAttackOverrideAnimEnd);
	AttackCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::AttackBoxOverlapBegin);
	
	EnableAttackCollisionBox(false);
	CrustyPirateGameInstance = Cast<UCrustyPirateGameInstance>(GetGameInstance());
 	if (CrustyPirateGameInstance)
	{
		HitPoints = CrustyPirateGameInstance->PlayerHP;

 		if (CrustyPirateGameInstance->IsDoubleJumpUnlocked)
 		{
 			UnlockDoubleJump();
 		}
	}

	if (PlayerHUDClass)
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0),
			PlayerHUDClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToPlayerScreen();

			PlayerHUDWidget->SetHP(HitPoints);
			PlayerHUDWidget->SetDiamonds(CrustyPirateGameInstance->CollectedDiamondCount);
			PlayerHUDWidget->SetLevel(CrustyPirateGameInstance->CurrentLevelIndex);
		}
	}
}
	
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::JumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::JumpEnded);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this, &APlayerCharacter::JumpEnded);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Attack);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	float MoveActionValue = Value.Get<float>();
	
	if (IsAlive && CanMove && !IsStunned)
	{
		FVector Direction = FVector::ForwardVector;
		AddMovementInput(Direction, MoveActionValue);
		UpdateDirection(MoveActionValue);
	}
}

void APlayerCharacter::UpdateDirection(float MoveDirection)
{
	FRotator CurrentRotation = Controller->GetControlRotation();

	if (MoveDirection < 0.0f)
	{
		if (CurrentRotation.Yaw != 180.0f)
		{
			Controller->SetControlRotation(FRotator(CurrentRotation.Pitch, 180.0f, CurrentRotation.Roll));
		}
	}
	else if (MoveDirection > 0.0f)
	{
		if (CurrentRotation.Yaw != 0.0f)
		{
			Controller->SetControlRotation(FRotator(CurrentRotation.Pitch, 0.0f, CurrentRotation.Roll));
		}
	}
}

void APlayerCharacter::OnAttackOverrideAnimEnd(bool Completed)
{
	if (IsActive  && IsAlive)
	{
		CanAttack = true;
		CanMove = true;
	}
}

 void APlayerCharacter::AttackBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
	{
		Enemy->TakeDamage(AttackDamage, AttackStunDuration);
	}
}

void APlayerCharacter::EnableAttackCollisionBox(bool Enabled)
{
	if (Enabled)
	{
		AttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttackCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
			ECollisionResponse::ECR_Overlap);
	}
	else
	{
		AttackCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttackCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
			ECollisionResponse::ECR_Ignore);
	}
}

void APlayerCharacter::JumpStarted(const FInputActionValue& Value)
{
	if (IsAlive && CanMove && !IsStunned)
	{
		Jump();
	}
}

void APlayerCharacter::JumpEnded(const FInputActionValue& Value)
{
	StopJumping();
}

void APlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (IsAlive && CanAttack && !IsStunned)
	{
		CanAttack = false;
		CanMove = false;

		GetAnimInstance()->PlayAnimationOverride(AttackAnimSequence, FName("DefaultSlot"), 1.0f,
			0.0f, OnAttackOverrideEndDelegate);
	}
}

void APlayerCharacter::TakeDamage(int DamageAmount, float StunDuration)
{
	if (!IsAlive) return;
	if (!IsActive) return;

	Stun(StunDuration);
	HitPoints = FMath::Clamp(HitPoints - DamageAmount, 0.0f, HitPoints);

	if (HitPoints > 0)
	{
		GetAnimInstance()->JumpToNode(FName("JumpTakeHit"), FName("CaptainStateMachine"));
	}
	else
	{
		Die();
	}
	
	UpdateHP(HitPoints);
}

void APlayerCharacter::UpdateHP(int NewHP)
{
	HitPoints = NewHP;
 	CrustyPirateGameInstance->SetPlayerHP(HitPoints);
	PlayerHUDWidget->SetHP(HitPoints);
}

void APlayerCharacter::Die()
{
	IsAlive = false;
	CanMove = false;
	CanAttack = false;

	EnableAttackCollisionBox(false);
	//HPText->SetHiddenInGame(true);
	GetAnimInstance()->JumpToNode(FName("JumpDie"), FName("CaptainStateMachine"));
	
	GetWorldTimerManager().SetTimer(DeathRestartTimer, this, &APlayerCharacter::OnDeathRestartTimerTimeout,
		1.0, false, DeathRestartWaitTime);
}

void APlayerCharacter::OnDeathRestartTimerTimeout()
{
	CrustyPirateGameInstance->RestartGame();
}

void APlayerCharacter::Stun(float DurationInSeconds)
{
	IsStunned = true;

	if (GetWorldTimerManager().IsTimerActive(StunTimer))
	{
		GetWorldTimerManager().ClearTimer(StunTimer);	
	}
	GetWorldTimerManager().SetTimer(StunTimer, this, &APlayerCharacter::OnStunTimerTimeout,
		1.0f, false, DurationInSeconds);

	// Halts player if mid attack animation
	GetAnimInstance()->StopAllAnimationOverrides();
	EnableAttackCollisionBox(false);
}

void APlayerCharacter::OnStunTimerTimeout()
{
	IsStunned = false;
}

void APlayerCharacter::CollectItem(CollectableType ItemType)
{
	UGameplayStatics::PlaySound2D(GetWorld(), ItemPickupSound);

	int HealAmount = 25;

	switch (ItemType)
	{
	case CollectableType::HealthPotion:
		UpdateHP(HitPoints + HealAmount);
		break;
	case CollectableType::Diamond:
		CrustyPirateGameInstance->AddDiamonds(1);
		PlayerHUDWidget->SetDiamonds(CrustyPirateGameInstance->CollectedDiamondCount);
		break;
	case CollectableType::DoubleJumpUpgrade:
		if (!CrustyPirateGameInstance->IsDoubleJumpUnlocked)
		{
			UnlockDoubleJump();
		}
		break;
	default:
			break;
	}
}

void APlayerCharacter::UnlockDoubleJump()
{
	JumpMaxCount = 2;
	CrustyPirateGameInstance->IsDoubleJumpUnlocked = true;
}

void APlayerCharacter::Deactivate()
{
	if (IsActive)
	{
		IsActive = false;
		CanAttack = false;
		CanMove = false;

		GetCharacterMovement()->StopMovementImmediately();		
	}
}
