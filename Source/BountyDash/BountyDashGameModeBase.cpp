// Copyright Epic Games, Inc. All Rights Reserved.


#include "BountyDashGameModeBase.h"
#include "BountyDashCharacter.h"
#include "BountyDashHUD.h"

ABountyDashGameModeBase::ABountyDashGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// setting default pawn class for ABountyDashGameModeBase
	DefaultPawnClass = ABountyDashCharacter::StaticClass();

	//기본 HUD 클래스 설정
	HUDClass = ABountyDashHUD::StaticClass();

	NumCoinsForSpeedIncrease = 5;
	GameSpeed = 10.0f;
	GameSpeedIncrease = 5.0f;
	GameLevel = 1;
	
	RunTime = 0.0f;
	bGameOver = false;
	startGameOverCount = false;
	timeTillGameOver =  2.0f;
	gameOverTimer =0.0f;
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

void ABountyDashGameModeBase::ReduceGameSpeed()
{
	if (GameSpeed > 10.0f)
	{
		GameSpeed -= GameSpeedIncrease;
		GameLevel--;
	}
}

float ABountyDashGameModeBase::GetRuntime()
{
	return RunTime;
}

void ABountyDashGameModeBase::Tick(float DeltaSeconds)
{
	if (!startGameOverCount)
	{
		RunTime += DeltaSeconds;
	}
	else
	{
		gameOverTimer += DeltaSeconds;
		if (gameOverTimer >= timeTillGameOver)
		{
			bGameOver = true;
		}
	}

}

bool ABountyDashGameModeBase::GetGameOver()
{
	return bGameOver;
}

void ABountyDashGameModeBase::GameOver()
{
	startGameOverCount = true;
}

void ABountyDashGameModeBase::SetGamePaused(bool gamePaused)
{
	APlayerController* myPlayer = GetWorld()->GetFirstPlayerController();
	if (myPlayer != nullptr)
	{
		myPlayer->SetPause(gamePaused);
	}
}
