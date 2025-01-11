#include "CrustyPirateGameInstance.h"

#include "Kismet/GameplayStatics.h"

void UCrustyPirateGameInstance::SetPlayerHP (int NewHP)
{
	PlayerHP = NewHP;
}

void UCrustyPirateGameInstance::AddDiamonds(int Amount)
{
	CollectedDiamondCount += Amount;
}

void UCrustyPirateGameInstance::ChangeLevel(int LevelIndex)
{
	if (LevelIndex <= 0) return;

	CurrentLevelIndex = LevelIndex;

	FString LevelNameString = FString::Printf(TEXT("Level_%02d"), CurrentLevelIndex);
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, LevelNameString);
	UGameplayStatics::OpenLevel(GetWorld(), FName(LevelNameString));
}

void UCrustyPirateGameInstance::RestartGame()
{
	PlayerHP = 100;
	CollectedDiamondCount = 0;
	IsDoubleJumpUnlocked = false;
	
	CurrentLevelIndex = 1;

	ChangeLevel(CurrentLevelIndex);
}
