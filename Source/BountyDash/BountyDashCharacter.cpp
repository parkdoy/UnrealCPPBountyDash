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
#include "Coin.h"
#include "DestructibleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Floor.h"
#include "BountyDashParticle.h"


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

		//게임속성
		CharSpeed = 10.0f;
		SmashTime = 10.0f;
		SmashTime = 10.0f;
		MagnetTime = 10.0f;
		MagnetReach = 1000.0f;

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

	//킬포인트 위치 설정
	for (TActorIterator<AFloor>TargetIter(GetWorld()); TargetIter; ++TargetIter)
	{
		KillPoint = TargetIter->GetKillPoint();

	}
}

// Called every frame
void ABountyDashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//라인 사이를 부드럽게 이동
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

	//장애물에 부딪혔을때 대상이 BeingPushed라면 케릭터를 뒤로 밀어낸다
	if (bBeingPushed)
	{
		float MoveSpeed = GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->GetInvGameSpeed();
		AddActorLocalOffset(FVector(MoveSpeed, 0.0f, 0.0f));

	}

	//코인을 자석처럼 끌어당긴다.
	if (CanMagnet)
	{
		CoinMagmet();
	}

	//KillPoint에 다다르면 GameOver 처리
	if (GetActorLocation().X < KillPoint)
	{
		GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->GameOver();
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

	//bExecuteWhenPaused - 게임이 일시 중지된 상태라도 인풋 액션을 수행할 수 있게 설정
	InputComponent->BindAction("Reset", IE_Pressed, this, &ABountyDashCharacter::Reset).bExecuteWhenPaused = true;
}

void ABountyDashCharacter::ScoreUp()
{
	Score++;
	GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->CharScoreUp(Score);
}

void ABountyDashCharacter::PowerUp(EPowerUp Type)
{
	switch (Type)
	{
	case EPowerUp::SPEED: 
	{
		GetCustomGameMode<ABountyDashGameModeBase>(GetWorld())->ReduceGameSpeed();
		break; 
	}
	case EPowerUp::SMASH:
	{
		CanSmash = true;
		FTimerHandle newTimer;
		GetWorld()->GetTimerManager().SetTimer(newTimer, this, &ABountyDashCharacter::StopSmash, SmashTime, false);
		break;
	}
	case EPowerUp::MAGNET:
	{
		CanMagnet = true;
		FTimerHandle newTimer;
		GetWorld()->GetTimerManager().SetTimer(newTimer, this, &ABountyDashCharacter::StopMagnet, MagnetTime, false);
		break;
	}
	default:
		break;
	}
}

int ABountyDashCharacter::GetScore()
{
	return Score;
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
		// 오브젝트를 부시거나 캐릭터 밀어내기 bBeingPushed 설정
		FVector VecBetween = OtherActor->GetActorLocation() - GetActorLocation();
		float AngleBetween = FMath::Acos(FVector::DotProduct(VecBetween.GetSafeNormal(), GetActorForwardVector().GetSafeNormal()));

		AngleBetween *= (180 / PI);

		if (AngleBetween < 60.0f)
		{
			AObstacle* pObs = Cast<AObstacle>(OtherActor);
			if(pObs && CanSmash)
			{
				pObs->GetDestructable()->ApplyRadiusDamage(10000, GetActorLocation(), 10000, 10000, true);

				// 파괴 이펙트 출력
				ABountyDashParticle* particleSys = GetWorld()->SpawnActor<ABountyDashParticle>(ABountyDashParticle::StaticClass(), GetTransform());
				particleSys->SetKillPoint(KillPoint);
			}
			else
			{
				bBeingPushed = true;
			}
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

void ABountyDashCharacter::StopSmash()
{
	CanMagnet = false;
}

void ABountyDashCharacter::StopMagnet()
{
	CanSmash = false;
}

void ABountyDashCharacter::CoinMagmet()
{
	for (TActorIterator<ACoin>coinIter(GetWorld()); coinIter; ++coinIter)
	{
		FVector between = GetActorLocation() - coinIter->GetActorLocation();
		if (FMath::Abs(between.Size()) < MagnetReach)
		{
			FVector CoinPos = FMath::Lerp((*coinIter)->GetActorLocation(), GetActorLocation(), 0.2);
			(*coinIter)->SetActorLocation(CoinPos);
			(*coinIter)->BeginPulled=true;
		}
	}
}

void ABountyDashCharacter::Reset()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("BountyDashMap"));
}