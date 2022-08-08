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
};
