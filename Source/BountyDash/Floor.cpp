// Fill out your copyright notice in the Description page of Project Settings.


#include "Floor.h"
#include "Components/BoxComponent.h"
#include "BountyDashGameModeBase.h"

// Sets default values
AFloor::AFloor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// finding resource for mesh and material
	ConstructorHelpers::FObjectFinder<UStaticMesh> MyMesh
	(TEXT("StaticMesh'/Game/BountyDash/Box_Brush2_StaticMesh.Box_Brush2_StaticMesh'"));
	ConstructorHelpers::FObjectFinder<UMaterial> MyMaterial
	(TEXT("Material'/Game/StarterContent/Materials/M_Concrete_Tiles.M_Concrete_Tiles'"));

	if (MyMesh.Succeeded())
	{
		NumRepeatingMesh = 3;

		// 
		FBoxSphereBounds MyBounds = MyMesh.Object->GetBounds();
		// length of floor
		float XBounds = MyBounds.BoxExtent.X * 2;
		// starting point of middle
		float ScenePos = ((XBounds * (NumRepeatingMesh - 1)) / 2.0f) * -1;
		// location for game over
		KillPoint = ScenePos - (XBounds * 0.5);
		// location for spawning gimmick
		SpawnPoint = (ScenePos * -1) + (XBounds * 0.5f);

		for (int i = 0; i < NumRepeatingMesh; i++)
		{
			FString SceneName = "Scene" + FString::FromInt(i);
			FName SceneID = FName(*SceneName);
			USceneComponent* ThisScene = CreateDefaultSubobject<USceneComponent>(SceneID);

			check(ThisScene);

			ThisScene->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			ThisScene->SetRelativeLocation(FVector(ScenePos, 0.0f, 0.0f));
			ScenePos += XBounds;

			FloorMeshScenes.Add(ThisScene);

			FString MeshName = "Mesh" + FString::FromInt(i);
			UStaticMeshComponent* ThisMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(*MeshName));

			check(ThisMesh);

			ThisMesh->AttachToComponent(FloorMeshScenes[i], FAttachmentTransformRules::KeepRelativeTransform);
			ThisMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			ThisMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

			if (MyMaterial.Succeeded())
			{
				ThisMesh->SetStaticMesh(MyMesh.Object);
				ThisMesh->SetMaterial(0, MyMaterial.Object);
			}

			FloorMeshes.Add(ThisMesh);
		}

		CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
		check(CollisionBox);

		CollisionBox->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		CollisionBox->SetBoxExtent(FVector(SpawnPoint, MyBounds.BoxExtent.Y, MyBounds.BoxExtent.Z));
		CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));


	}
}

// Called when the game starts or when spawned
void AFloor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (auto Scene : FloorMeshScenes)
	{
		Scene->AddLocalOffset(FVector(GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->GetInvGameSpeed(), 0.0f, 0.0f));

		if (Scene->GetComponentTransform().GetLocation().X <= KillPoint)
		{
			Scene->SetRelativeLocation(FVector(SpawnPoint, 0.0f, 0.0f));
		}
	}
}

float AFloor::GetKillPoint()
{
	return KillPoint;
}

float AFloor::GetSpawnPoint()
{
	return SpawnPoint;
}

