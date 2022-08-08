// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BountyDash.h"
#include "GameFramework/Character.h"
#include "BountyDashCharacter.generated.h"

UCLASS()
class BOUNTYDASH_API ABountyDashCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABountyDashCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, Category = Logic)
	TArray<class ATargetPoint*> TargetArray;

	UPROPERTY(EditAnywhere, Category = Logic)
	float CharSpeed;

	UPROPERTY(EditAnywhere, Category = Sound)
	UAudioComponent* HitObstacleSound;

	UPROPERTY(EditAnywhere, Category = Sound)
	UAudioComponent* DingSound;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(BlueprintReadOnly)
	int32 Score;

private:
	// Location where character is at
	short CurrentLocation;
	FVector DesiredLocation;
	bool bBeingPushed;

public:
	void ScoreUp();

protected:
	// movement of character -> left and right
	void MoveRight();
	void MoveLeft();

	// function for collision
	UFUNCTION()
	void MyOnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void MyOnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
