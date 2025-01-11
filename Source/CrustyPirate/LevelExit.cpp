#include "LevelExit.h"

#include "PlayerCharacter.h"
#include "CrustyPirateGameInstance.h"

#include "Kismet/GameplayStatics.h"

ALevelExit::ALevelExit()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	
	DoorFlipbook = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("DoorFlipbook"));
	DoorFlipbook->SetupAttachment(RootComponent);
	DoorFlipbook->SetPlayRate(0.0f);
	DoorFlipbook->SetLooping(false);
}

void ALevelExit::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ALevelExit::OverlapBegin);

	DoorFlipbook->SetPlaybackPosition(0.0f, false);
}

void ALevelExit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelExit::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		if (Player->IsAlive)
		{
			if (IsActive)
			{
				Player->Deactivate();
				
				IsActive = false;

				DoorFlipbook->SetPlayRate(1.0f);
				DoorFlipbook->PlayFromStart();

				UGameplayStatics::PlaySound2D(GetWorld(), PlayerEnterSound);

				GetWorldTimerManager().SetTimer(WaitTimer, this, &ALevelExit::OnWaitTimerTimeout,
					1.0f, false, WaitTimeInSeconds);
			}
		}
	}
}

void ALevelExit::OnWaitTimerTimeout()
{
	if (UCrustyPirateGameInstance* GameInstance = Cast<UCrustyPirateGameInstance>(GetGameInstance()))
	{
		GameInstance->ChangeLevel(LevelIndex);
	}
}

