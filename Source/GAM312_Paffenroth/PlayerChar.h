#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Resource_M.h"
#include "Kismet/GameplayStatics.h"
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

	// Property to allow the setting of player health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Health = 100.0f;

	// Property to allow the setting of player hunger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Hunger = 100.0f;

	// Property to allow the setting of player stamina
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
		float Stamina = 100.0f;

	// Resource counts for each resource
	UPROPERTY(EditAnywhere, Category = "Resources")
		int Wood = 0;

	UPROPERTY(EditAnywhere, Category = "Resources")
		int Stone = 0;

	UPROPERTY(EditAnywhere, Category = "Resources")
		int Berry = 0;

	// Dynamic array tracking amounts of each resource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
		TArray<int> ResourcesArray;

	// Names of each resource type
	UPROPERTY(EditAnywhere, Category = "Resources")
		TArray<FString> ResourcesNameArray;

	// Material for spawning the hit decal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitMarker")
		UMaterialInterface* hitDecal;
	
	// Adjusts the player health by a given amount
	UFUNCTION(BlueprintCallable)
		void SetHealth(float amount);

	// Adjusts the player hunger by a given amount
	UFUNCTION(BlueprintCallable)
		void SetHunger(float amount);

	// Adjusts the player stamina by a given amount
	UFUNCTION(BlueprintCallable)
		void SetStamina(float amount);

	// Decreases player stats over time
	UFUNCTION()
		void DecreaseStats();

	// Adds a specific resource type and amount to the player's inventory
	UFUNCTION()
		void GiveResource(int32 amount, FString resourceType);
};
