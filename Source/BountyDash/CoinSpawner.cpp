// Fill out your copyright notice in the Description page of Project Settings.


#include "CoinSpawner.h"
#include "EngineUtils.h"
#include "Floor.h"
#include "Engine/TargetPoint.h"
#include "Coin.h"
#include "Components/SphereComponent.h"
#include "BountyDashPowerUp.h"


// Sets default values
ACoinSpawner::ACoinSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MaxSetCoins = 5;
	MinSetCoins = 3;
	CoinSetTimeInterval = 4.0f;
	CoinTimeInterval = 0.5f;
	MovementTimeInterval = 1.0f;
}

// Called when the game starts or when spawned
void ACoinSpawner::BeginPlay()
{
	Super::BeginPlay();

	// get all actor of class (ATargetPoint)
	for (TActorIterator<ATargetPoint> TargetIter(GetWorld()); TargetIter; ++TargetIter)
	{
		SpawnTransforms.Add(*TargetIter);
	}

	// get all actor of class (AFloor)
	for (TActorIterator<AFloor> FloorIter(GetWorld()); FloorIter; ++FloorIter)
	{
		if (FloorIter->GetWorld() == GetWorld())
		{
			KillPoint = FloorIter->GetKillPoint();
			SpawnPoint = FloorIter->GetSpawnPoint();
		}
	}

	// set timer by function name
	FTimerManager& WorldTimeManager = GetWorld()->GetTimerManager();

	WorldTimeManager.SetTimer(CoinSetTimerHandle, this, &ACoinSpawner::SpawnCoin, CoinSetTimeInterval, false);
	WorldTimeManager.SetTimer(SpawnMoveTimerHandle, this, &ACoinSpawner::MoveSpawner, MovementTimeInterval, true);
}


// Called every frame
void ACoinSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACoinSpawner::SpawnCoin()
{
	if (FMath::Rand() % 100 < PowerUpChance)
	{
		SpawnPowerUp();
		NumCoinsToSpawn--;
	}
	else
	{//코인 스폰
		FActorSpawnParameters SpawnParams;

		FTransform CoinTransform = SpawnTransforms[TargetLoc]->GetTransform();

		CoinTransform.SetLocation(FVector(SpawnPoint, CoinTransform.GetLocation().Y, CoinTransform.GetLocation().Z));

		ACoin* SpawnedCoin = GetWorld()->SpawnActor<ACoin>(CoinObject, CoinTransform, SpawnParams);

		if (SpawnedCoin)
		{
			USphereComponent* CoinSphere = Cast<USphereComponent>(SpawnedCoin->GetComponentByClass(USphereComponent::StaticClass()));

			if (CoinSphere)
			{
				float Offset = CoinSphere->GetUnscaledSphereRadius();
				SpawnedCoin->AddActorLocalOffset(FVector(0.0f, 0.0f, Offset));

				NumCoinsToSpawn--;
			}
		}
	}

	if (NumCoinsToSpawn <= 0)
	{
		FTimerManager& WorldTimeManager = GetWorld()->GetTimerManager();
		WorldTimeManager.SetTimer(CoinSetTimerHandle, this, &ACoinSpawner::SpawnCoinSet, CoinSetTimeInterval,false);
		WorldTimeManager.ClearTimer(CoinTimerHandle);
	}
}

void ACoinSpawner::SpawnCoinSet()
{
	NumCoinsToSpawn = FMath::RandRange(MinSetCoins, MaxSetCoins);

	FTimerManager& WorldTimeManager = GetWorld()->GetTimerManager();

	// change activated timer
	WorldTimeManager.ClearTimer(CoinSetTimerHandle);
	WorldTimeManager.SetTimer(CoinTimerHandle, this, &ACoinSpawner::SpawnCoin, CoinTimeInterval, true);
}

void ACoinSpawner::MoveSpawner()
{
	TargetLoc = FMath::Rand() % SpawnTransforms.Num();
}

void ACoinSpawner::SpawnPowerUp()
{
	FActorSpawnParameters SpawnInfo;

	FTransform myTrans = SpawnTransforms[TargetLoc]->GetTransform();
	myTrans.SetLocation(FVector(SpawnPoint, myTrans.GetLocation().Y, myTrans.GetLocation().Z));

	ABountyDashPowerUp* newObs = GetWorld()->SpawnActor < ABountyDashPowerUp>(PowerUpObject, myTrans, SpawnInfo);
	
	if (newObs)
	{
		newObs->SetKillPoint(KillPoint);
		USphereComponent* powerUpSphere = Cast<USphereComponent>(newObs->GetComponentByClass(USphereComponent::StaticClass()));

		if (powerUpSphere)
		{
			float offset = powerUpSphere->GetUnscaledSphereRadius();
			newObs->AddActorLocalOffset(FVector(0.0f, 0.0f, offset));
		}
	}
}

