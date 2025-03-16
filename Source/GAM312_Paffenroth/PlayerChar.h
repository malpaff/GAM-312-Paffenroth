// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "PlayerChar.generated.h"

UCLASS()
class GAM312_PAFFENROTH_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Function to move the player character forward and backward
	UFUNCTION()
		void MoveForward(float axisValue);

	// Function to move the player character left and right
	UFUNCTION()
		void MoveRight(float axisValue);

	// Function to start the jumping animation when a button is pressed
	UFUNCTION()
		void StartJump();

	// Function to stop the jumping animation when a button is released
	UFUNCTION()
		void StopJump();

	// Function placeholder for object interaction
	UFUNCTION()
		void FindObject();

	// Camera component that provides the player's viewpoint
	UPROPERTY(VisibleAnywhere)
		UCameraComponent* PlayerCamComp;
};
