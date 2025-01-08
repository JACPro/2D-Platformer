#include "Enemy.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerDetectorSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectorSphere"));
	PlayerDetectorSphere->SetupAttachment(RootComponent);

	HPText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HPText"));
	HPText->SetupAttachment(RootComponent);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	PlayerDetectorSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::DetectorOverlapBegin);
	PlayerDetectorSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::DetectorOverlapEnd);

	UpdateHP(HitPoints);

	OnAttackOverrideEndDelegate.BindUObject(this, &AEnemy::OnAttackOverrideAnimEnd);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive && FollowTarget && !IsStunned)
	{
		float MoveDirection = (FollowTarget->GetActorLocation().X - GetActorLocation().X) > 0.0f ? 1.0f : -1.0f;
		UpdateDirection(MoveDirection);
		
		if (ShouldMoveToTarget())
		{
			if (CanMove)
			{
				FVector WorldDirection = FVector::ForwardVector;
				AddMovementInput(WorldDirection, MoveDirection);
			}
		}
		else
		{
			if (FollowTarget->IsAlive)
			{
				Attack();
			}
		}
	}
}

void AEnemy::DetectorOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		FollowTarget = Player;
	}
}

void AEnemy::DetectorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		FollowTarget = NULL;
	}
}

bool AEnemy::ShouldMoveToTarget()
{
	bool Result = false;

	if (FollowTarget)
	{
		float DistanceToTarget = abs(FollowTarget->GetActorLocation().X - GetActorLocation().X);
		Result = DistanceToTarget > StopDistanceToTarget;
	}
	
	return Result;
}

void AEnemy::UpdateDirection(float MoveDirection)
{
	FRotator CurrentRotation = GetActorRotation();

	if (MoveDirection < 0.0f)
	{
		if (CurrentRotation.Yaw != 180.0f)
		{
			SetActorRotation(FRotator(CurrentRotation.Pitch, 180.0f, CurrentRotation.Roll));
		}
	}
	else if (MoveDirection > 0.0f)
	{
		if (CurrentRotation.Yaw != 0.0f)
		{
			SetActorRotation(FRotator(CurrentRotation.Pitch, 0.0f, CurrentRotation.Roll));
		}
	}
}

void AEnemy::UpdateHP(int NewHP)
{
	HitPoints = NewHP;

	FString Str = FString::Printf(TEXT("HP: %d"), HitPoints);
	HPText->SetText(FText::FromString(Str));
}

void AEnemy::TakeDamage(int DamageAmount, float StunDuration)
{
	if (!IsAlive) return;

	Stun(StunDuration);
	HitPoints = FMath::Clamp(HitPoints - DamageAmount, 0.0f, HitPoints);
	
	if (HitPoints > 0.0f)
	{
		GetAnimInstance()->JumpToNode(FName("JumpTakeHit"), FName("CrabbyStateMachine"));
	}
	else
	{
		Die();
	}
	
	UpdateHP(HitPoints);
}

void AEnemy::Die()
{
	IsAlive = false;
	CanMove = false;
	CanAttack = false;
	
	HPText->SetHiddenInGame(true);
	GetAnimInstance()->JumpToNode(FName("JumpDie"), FName("CrabbyStateMachine"));
}

void AEnemy::Stun(float DurationInSeconds)
{
	IsStunned = true;

	if (GetWorldTimerManager().IsTimerActive(StunTimer))
	{
		GetWorldTimerManager().ClearTimer(StunTimer);	
	}
	GetWorldTimerManager().SetTimer(StunTimer, this, &AEnemy::OnStunTimerTimeout,
		1.0f, false, DurationInSeconds);

	// Halts enemy if mid attack animation
	GetAnimInstance()->StopAllAnimationOverrides();
}

void AEnemy::OnStunTimerTimeout()
{
	IsStunned = false;
}

void AEnemy::Attack()
{
	if (IsAlive && CanAttack && !IsStunned)
	{
		CanAttack = false;
		CanMove = false;

		GetAnimInstance()->PlayAnimationOverride(AttackAnimSequence, FName("DefaultSlot"), 1.0f,
	0.0f, OnAttackOverrideEndDelegate);

		GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &AEnemy::OnAttackCooldownTimerTimeout,
		1.0f, false, AttackCooldownInSeconds);

	}
}

void AEnemy::OnAttackOverrideAnimEnd(bool Completed)
{
	if (IsAlive)
	{
		CanMove = true;
	}
}

void AEnemy::OnAttackCooldownTimerTimeout()
{
	if (IsAlive)
	{
		CanAttack = true;
	}
}

