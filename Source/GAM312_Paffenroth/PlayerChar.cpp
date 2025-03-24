#include "PlayerChar.h"
#include "DrawDebugHelpers.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create amd attach the player's camera to the head socket
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));
	PlayerCamComp->SetupAttachment(GetMesh(), FName("head"));
	PlayerCamComp->bUsePawnControlRotation = true;

	// Initializes resource tracking arrays
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));
}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// Starts a repeating timer that decreases stats every 2 seconds
	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);
	
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
}

// Function that handles forward and backward movement
void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

// Function that handles the right and left movement
void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

// Function that starts the jumping action
void APlayerChar::StartJump()
{
	bPressedJump = true;
}

// Function that stops the jumping action
void APlayerChar::StopJump()
{
	bPressedJump = false;
}

// Function that handles interaction with resources using a line trace
void APlayerChar::FindObject()
{
	FHitResult HitResult;

	// Define start and end points of the line trace from the player's camera
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;
	// Visual line trace used for debugging
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 2.0f);

	// Set up collision query to ignore the player
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	// Perform the line trace
	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		// Check if the hit actor is a resource
		AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

		// Check if the player has enough stamina to gather the resource
		if (Stamina >= 5.0f)
		{
			if (HitResource)
			{
				FString hitName = HitResource->resourceName;
				int resourceValue = HitResource->resourceAmount;

				// Reduces the resource's total amount
				HitResource->totalResource = HitResource->totalResource - resourceValue;

				// If the resource still has enough total amount remaining, give the resource to the player
				if (HitResource->totalResource >= resourceValue)
				{
					GiveResource(resourceValue, hitName);

					// Debug message for resource collection
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Collected"));

					// Spawn a decal at the hit location
					UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

					// Decrease stamina while collecting
					SetStamina(-5.0f);
				}
				else
				{
					// Resource is depleted, destroy the actor
					HitResource->Destroy();
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Depleted"));
				}
			}
		}
	}
}

// Adjusts player health by a set amount, clamped between 0 and 100
void APlayerChar::SetHealth(float amount)
{
	Health = FMath::Clamp(Health + amount, 0.0f, 100.0f);
}

// Adjusts player hunger by the set amount, clamped between 0 and 100
void APlayerChar::SetHunger(float amount)
{
	Hunger = FMath::Clamp(Hunger + amount, 0.0f, 100.0f);
}

// Adjusts player stamina by the set amount, clamed between 0 and 100
void APlayerChar::SetStamina(float amount)
{
	Stamina = FMath::Clamp(Stamina + amount, 0.0f, 100.0f);
}

// Decreases player stats over time
void APlayerChar::DecreaseStats()
{
	// Gradually reduce hunger
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}

	// Gradually restore stamina
	SetStamina(10.0f);

	// If hunger runs out, start reducing health
	if (Hunger <= 0)
	{
		SetHealth(-3.0f);
	}

	// Display stamina for debugging
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Stamina: %f"), Stamina));
	}
}

// Adds collected resources to the inventory based on the resource type
void APlayerChar::GiveResource(int32 amount, FString resourceType)
{
	// Ensure resource array is the proper size
	if (ResourcesArray.Num() >= 3)
	{
		// Match the resources type to the correct slot
		if (resourceType == "Wood")
		{
			ResourcesArray[0] += amount;
		}
		else if (resourceType == "Stone")
		{
			ResourcesArray[1] += amount;
		}

		if (resourceType == "Berry")
		{
			ResourcesArray[2] += amount;
		}
	}
}

