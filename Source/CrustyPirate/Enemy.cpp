#include "Enemy.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerDetectorSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerDetectorSphere"));
	PlayerDetectorSphere->SetupAttachment(RootComponent);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	PlayerDetectorSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::DetectorOverlapBegin);
	PlayerDetectorSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::DetectorOverlapEnd);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive && FollowTarget)
	{
		float MoveDirection = (FollowTarget->GetActorLocation().X - GetActorLocation().X) > 0.0f ? 1.0f : -1.0f;

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
			//Attack();
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
