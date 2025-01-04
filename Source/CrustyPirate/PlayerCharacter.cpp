#include "PlayerCharacter.h"

APlayerCharacter::APlayerCharacter() 
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
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
	
	if (IsAlive && CanMove)
	{
		FVector Direction = FVector::ForwardVector;
		AddMovementInput(Direction, MoveActionValue);	
	}
}

void APlayerCharacter::JumpStarted(const FInputActionValue& Value)
{
	if (IsAlive && CanMove)
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
}
