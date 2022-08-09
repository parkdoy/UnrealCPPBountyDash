// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BountyDashGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BOUNTYDASH_API ABountyDashGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ABountyDashGameModeBase();

	void CharScoreUp(unsigned int CharScore);

	UFUNCTION()
	float GetInvGameSpeed();

	UFUNCTION()
	float GetGameSpeed();

	UFUNCTION()
	int32 GetGameLevel();

	void ReduceGameSpeed();

	float GetRuntime();
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
		bool GetGameOver();

	UFUNCTION()
		void GameOver();

	UFUNCTION()
		void SetGamePaused(bool gamePaused);


private:
	UPROPERTY()
	float GameSpeed;

	UPROPERTY()
	int32 GameLevel;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 NumCoinsForSpeedIncrease;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameSpeedIncrease;

	UPROPERTY()
	float RunTime;
	
	UPROPERTY()
		bool bGameOver;

	UPROPERTY()
		bool startGameOverCount;

	UPROPERTY()
		float timeTillGameOver;

	UPROPERTY()
		float gameOverTimer;
};
