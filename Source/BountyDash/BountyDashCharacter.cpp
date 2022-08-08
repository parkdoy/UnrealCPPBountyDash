// Fill out your copyright notice in the Description page of Project Settings.


#include "BountyDashCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Obstacle.h"
#include "BountyDashGameModeBase.h"

// Sets default values
ABountyDashCharacter::ABountyDashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// collision capsule's size
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// getting information of skeletal mesh and animation blueprint from asset storage
	ConstructorHelpers::FObjectFinder<UAnimBlueprint> MyAnimBP
	(TEXT("AnimBlueprint'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP'"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> MySkeletalMesh
	(TEXT("SkeletalMesh'/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));

	if (MyAnimBP.Succeeded() && MySkeletalMesh.Succeeded())
	{
		// setting info of skeletal mesh and its animation blueprint
		GetMesh()->SetSkeletalMesh(MySkeletalMesh.Object);
		GetMesh()->SetAnimInstanceClass(MyAnimBP.Object->GeneratedClass);

		// mesh's location and rotation
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

		GetCharacterMovement()->JumpZVelocity = 1450.0f;
		GetCharacterMovement()->GravityScale = 4.5f;

		// creating camera boom
		CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
		
		// statement like isValid
		check(CameraBoom);

		CameraBoom->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		
		// Setting distance between camera and character
		CameraBoom->TargetArmLength = 500.0f;

		// offset
		CameraBoom->AddRelativeLocation(FVector(0.0f, 0.0f, 160.0f));

		// creating camera
		FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
		check(FollowCamera);

		// attach camera to cameraboom
		FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepRelativeTransform);

		// setting rotation of camera to look at character slightly downward
		FollowCamera->AddRelativeRotation(FQuat(FRotator(-10.0f, 0.0f, 0.0f)));

		CharSpeed = 10.0f;

		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABountyDashCharacter::MyOnComponentBeginOverlap);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABountyDashCharacter::MyOnComponentEndOverlap);

		// Get input info of player 0
		AutoPossessPlayer = EAutoReceiveInput::Player0;
	}

}

// Called when the game starts or when spawned
void ABountyDashCharacter::BeginPlay()
{
	Super::BeginPlay();

	// find all the target points placed in the wolrd and put them in a array (TargetArray)
	for (TActorIterator<ATargetPoint> TargetIter(GetWorld()); TargetIter; ++TargetIter)
	{
		TargetArray.Add(*TargetIter);
	}

	// sorting based on the leftmost Targetpoint
	auto SortPred = [](const AActor& A, AActor& B)->bool
	{
		return (A.GetActorLocation().Y < B.GetActorLocation().Y);
	};
	TargetArray.Sort(SortPred);

	// Among Targetpoints, finding the middle one
	CurrentLocation = (TargetArray.Num() / 2 + TargetArray.Num() % 2 - 1);

}

// Called every frame
void ABountyDashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetArray.Num() > 0)
	{
		FVector targetLoc = TargetArray[CurrentLocation]->GetActorLocation();
		targetLoc.Z = GetActorLocation().Z;
		targetLoc.X = GetActorLocation().X;

		if (targetLoc != GetActorLocation())
		{
			SetActorLocation(FMath::Lerp(GetActorLocation(), targetLoc, CharSpeed * DeltaTime));
		}
	}

	if (bBeingPushed)
	{
		float MoveSpeed = GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->GetInvGameSpeed();
		AddActorLocalOffset(FVector(MoveSpeed, 0.0f, 0.0f));

	}
}

// Called to bind functionality to input
void ABountyDashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAction("GoRight", IE_Pressed, this, &ABountyDashCharacter::MoveRight);
	InputComponent->BindAction("GoLeft", IE_Pressed, this, &ABountyDashCharacter::MoveLeft);
	
}

void ABountyDashCharacter::ScoreUp()
{
	Score++;
	GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->CharScoreUp(Score);
}

void ABountyDashCharacter::MoveRight()
{
	if (Controller != nullptr)
	{
		if (CurrentLocation < TargetArray.Num() - 1)
		{
			++CurrentLocation;
		}
		//else
		//{
		//	// do nothing
		//}
	}
}

void ABountyDashCharacter::MoveLeft()
{
	if (Controller != nullptr)
	{
		if (CurrentLocation > 0)
		{
			--CurrentLocation;
		}
		//else
		//{
		//	// do nothing
		//}
	}
}

void ABountyDashCharacter::MyOnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->GetClass()->IsChildOf(AObstacle::StaticClass()))
	{
		FVector VecBetween = OtherActor->GetActorLocation() - GetActorLocation();
		float AngleBetween = FMath::Acos(FVector::DotProduct(VecBetween.GetSafeNormal(), GetActorForwardVector().GetSafeNormal()));

		AngleBetween *= (180 / PI);

		if (AngleBetween < 60.0f)
		{
			bBeingPushed = true;
		}
	}
}

void ABountyDashCharacter::MyOnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->GetClass()->IsChildOf(AObstacle::StaticClass()))
	{
		bBeingPushed = false;
	}
}

