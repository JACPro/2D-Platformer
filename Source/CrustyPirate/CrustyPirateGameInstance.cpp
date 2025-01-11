#include "CrustyPirateGameInstance.h"

void UCrustyPirateGameInstance::SetPlayerHP (int NewHP)
{
	PlayerHP = NewHP;
}

void UCrustyPirateGameInstance::AddDiamonds(int Amount)
{
	CollectedDiamondCount += Amount;
}
