// Copyright Epic Games, Inc. All Rights Reserved.


#include "BountyDashGameModeBase.h"
#include "BountyDashCharacter.h"

ABountyDashGameModeBase::ABountyDashGameModeBase()
{
	// setting default pawn class for ABountyDashGameModeBase
	DefaultPawnClass = ABountyDashCharacter::StaticClass();

	NumCoinsForSpeedIncrease = 5;
	GameSpeed = 10.0f;
	GameSpeedIncrease = 5.0f;
	GameLevel = 1;


}

void ABountyDashGameModeBase::CharScoreUp(unsigned int CharScore)
{
	if (CharScore != 0 && CharScore % NumCoinsForSpeedIncrease == 0)
	{
		GameSpeed += GameSpeedIncrease;
		GameLevel++;
	}
}

float ABountyDashGameModeBase::GetInvGameSpeed()
{
	return -GameSpeed;
}

float ABountyDashGameModeBase::GetGameSpeed()
{
	return GameSpeed;
}

int32 ABountyDashGameModeBase::GetGameLevel()
{
	return GameLevel;
}
