#include "PlayerChar.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "BuildingPart.h"
#include "EngineUtils.h"

// Helpers

static FVector GetMeshExtentsWS(const ABuildingPart* Part)
{
	if (!Part || !Part->Mesh) return FVector(50.f, 50.f, 50.f);
	return Part->Mesh->Bounds.BoxExtent; 
}

static bool OverlapsBlocking(UWorld* World, const AActor* IgnoredA, const AActor* IgnoredB, const FTransform& T, const FVector& HalfExtents)
{
	if (!World) return true;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(IgnoredA);
	if (IgnoredB) Params.AddIgnoredActor(IgnoredB);

	const FCollisionShape Box = FCollisionShape::MakeBox(HalfExtents);

	return World->OverlapAnyTestByChannel(
		T.GetLocation(),
		T.GetRotation(),
		ECC_WorldStatic,
		Box,
		Params
	);
}

static ABuildingPart* FindNearestPartOfType(UWorld* World, const FVector& Point, EBuildingPartType Type, float Radius, const AActor* IgnoreA, const AActor* IgnoreB)
{
	if (!World) return nullptr;

	ABuildingPart* Best = nullptr;
	float BestDistSq = FMath::Square(Radius);

	for (TActorIterator<ABuildingPart> It(World); It; ++It)
	{
		ABuildingPart* P = *It;
		if (!P || P == IgnoreA || P == IgnoreB) continue;
		if (P->PartType != Type) continue;

		const float D = FVector::DistSquared(P->GetActorLocation(), Point);
		if (D < BestDistSq)
		{
			BestDistSq = D;
			Best = P;
		}
	}
	return Best;
}

// Floors snap to other floors
static FVector SnapFloorToFloor(const ABuildingPart* TargetFloor, const FVector& PointWS, const FVector& MyExtentsWS)
{
	const FTransform TF = TargetFloor->GetActorTransform();
	const FVector Local = TF.InverseTransformPosition(PointWS);
	const FVector TargetExt = GetMeshExtentsWS(TargetFloor);

	const float DistToXEdge = TargetExt.X - FMath::Abs(Local.X);
	const float DistToYEdge = TargetExt.Y - FMath::Abs(Local.Y);

	FVector SnappedLocal = FVector::ZeroVector;

	if (DistToXEdge < DistToYEdge)
	{
		const float Sign = (Local.X >= 0.f) ? 1.f : -1.f;
		SnappedLocal.X = Sign * (TargetExt.X + MyExtentsWS.X);
		SnappedLocal.Y = 0.f;
	}
	else
	{
		const float Sign = (Local.Y >= 0.f) ? 1.f : -1.f;
		SnappedLocal.Y = Sign * (TargetExt.Y + MyExtentsWS.Y);
		SnappedLocal.X = 0.f;
	}

	SnappedLocal.Z = 0.f;
	return TF.TransformPosition(SnappedLocal);
}

// Walls snap to nearest floor edge
static FTransform SnapWallToFloor(const ABuildingPart* Floor, const FVector& PointWS, const FVector& WallExtWS)
{
	const FTransform TF = Floor->GetActorTransform();
	const FVector Local = TF.InverseTransformPosition(PointWS);
	const FVector FloorExt = GetMeshExtentsWS(Floor);

	const float DistToXEdge = FloorExt.X - FMath::Abs(Local.X);
	const float DistToYEdge = FloorExt.Y - FMath::Abs(Local.Y);

	FVector WallLocal(0.f, 0.f, 0.f);
	FRotator WallRot = TF.Rotator();
	WallRot.Pitch = 0.f;
	WallRot.Roll = 0.f;

	if (DistToXEdge < DistToYEdge)
	{
		// East/West edge
		const float Sign = (Local.X >= 0.f) ? 1.f : -1.f;
		WallLocal.X = Sign * FloorExt.X; 
		WallLocal.Y = 0.f;
		WallRot.Yaw += 90.f;
	}
	else
	{
		// North/South edge
		const float Sign = (Local.Y >= 0.f) ? 1.f : -1.f;
		WallLocal.Y = Sign * FloorExt.Y; 
		WallLocal.X = 0.f;
	}

	const float FloorTopWS = Floor->GetActorLocation().Z + FloorExt.Z;

	constexpr float WallZBias = 15.0f;

	const float WallCenterZ = FloorTopWS + WallExtWS.Z - WallZBias;

	const FVector WallCenterWS = TF.TransformPosition(WallLocal);

	FTransform Out;
	Out.SetLocation(FVector(WallCenterWS.X, WallCenterWS.Y, WallCenterZ));
	Out.SetRotation(WallRot.Quaternion());
	Out.SetScale3D(FVector(1.f));
	return Out;
}

// APlayerChar

APlayerChar::APlayerChar()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Cam"));
	PlayerCamComp->SetupAttachment(GetMesh(), FName("head"));
	PlayerCamComp->bUsePawnControlRotation = true;

	BuildingArray.SetNum(3);
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));
}

void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 1.0f, true);

	if (objWidget)
	{
		objWidget->UpdatebuildObj(0.0f);
		objWidget->UpdatematOBJ(0.0f);
	}
}

void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (playerUI)
	{
		playerUI->UpdateBars(Health, Hunger, Stamina);
	}

	if (isBuilding && spawnedPart)
	{
		const float TraceDist = 2000.f;
		const float SnapRadius = 300.f;

		const FVector CamStart = PlayerCamComp->GetComponentLocation();
		const FVector CamEnd = CamStart + PlayerCamComp->GetForwardVector() * 800.f;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(spawnedPart);
		Params.bTraceComplex = false;

		const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CamStart, CamEnd, ECC_Visibility, Params);
		const FVector AimPoint = bHit ? Hit.Location : (CamStart + PlayerCamComp->GetForwardVector() * 400.f);

		const EBuildingPartType MyType = spawnedPart->PartType;
		const FVector MyExt = GetMeshExtentsWS(spawnedPart);

		FTransform DesiredT = spawnedPart->GetActorTransform();
		DesiredT.SetScale3D(spawnedPart->GetActorScale3D());

		bool bValid = true;

		// ---------- FLOORS ----------
		if (MyType == EBuildingPartType::Floor)
		{
			FHitResult GroundHit;
			const FVector GroundStart(AimPoint.X, AimPoint.Y, AimPoint.Z + 500.f);
			const FVector GroundEnd(AimPoint.X, AimPoint.Y, AimPoint.Z - TraceDist);

			const bool bGround = GetWorld()->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_Visibility, Params);

			FVector FloorCenter = bGround ? GroundHit.Location : AimPoint;
			FloorCenter.Z += MyExt.Z;

			// Snap floor to floor
			if (ABuildingPart* NearFloor = FindNearestPartOfType(GetWorld(), FloorCenter, EBuildingPartType::Floor, SnapRadius, this, spawnedPart))
			{
				const FVector Snapped = SnapFloorToFloor(NearFloor, FloorCenter, MyExt);
				FloorCenter.X = Snapped.X;
				FloorCenter.Y = Snapped.Y;
				FloorCenter.Z = NearFloor->GetActorLocation().Z; 
			}

			FRotator R = spawnedPart->GetActorRotation();
			R.Pitch = 0.f;
			R.Roll = 0.f;
			DesiredT.SetRotation(R.Quaternion());
			DesiredT.SetLocation(FloorCenter);

			if (bGround)
			{
				const float BottomZ = FloorCenter.Z - MyExt.Z;
				const float Penetration = GroundHit.Location.Z - BottomZ;
				const float MaxAllowedPenetration = 10.f;
				if (Penetration > MaxAllowedPenetration)
				{
					bValid = false;
				}
			}

			// Overlap check
			if (OverlapsBlocking(GetWorld(), this, spawnedPart, DesiredT, MyExt * 0.98f))
			{
				bValid = false;
			}
		}

		// WALLS 
		else if (MyType == EBuildingPartType::Wall)
		{
			ABuildingPart* NearFloor = FindNearestPartOfType(GetWorld(), AimPoint, EBuildingPartType::Floor, SnapRadius, this, spawnedPart);
			if (!NearFloor)
			{
				bValid = false;
				DesiredT.SetLocation(AimPoint);
			}
			else
			{
				DesiredT = SnapWallToFloor(NearFloor, AimPoint, MyExt);
				DesiredT.SetScale3D(spawnedPart->GetActorScale3D());

				if (OverlapsBlocking(GetWorld(), this, spawnedPart, DesiredT, MyExt * 0.98f))
				{
					bValid = false;
				}
			}
		}

		// CEILINGS
		else if (MyType == EBuildingPartType::Ceiling)
		{
			ABuildingPart* NearWall = FindNearestPartOfType(GetWorld(), AimPoint, EBuildingPartType::Wall, SnapRadius, this, spawnedPart);
			ABuildingPart* NearFloor = FindNearestPartOfType(GetWorld(), AimPoint, EBuildingPartType::Floor, SnapRadius, this, spawnedPart);

			if (!NearWall || !NearFloor)
			{
				bValid = false;
				DesiredT.SetLocation(AimPoint);
			}
			else
			{
				const FVector WallExt = GetMeshExtentsWS(NearWall);
				const float WallTopWS = NearWall->GetActorLocation().Z + WallExt.Z;

				FVector CeilingCenter = NearFloor->GetActorLocation();
				CeilingCenter.Z = WallTopWS + MyExt.Z;

				FRotator R = NearWall->GetActorRotation();
				R.Pitch = 0.f;
				R.Roll = 0.f;

				DesiredT.SetLocation(CeilingCenter);
				DesiredT.SetRotation(R.Quaternion());
				DesiredT.SetScale3D(spawnedPart->GetActorScale3D());

				if (OverlapsBlocking(GetWorld(), this, spawnedPart, DesiredT, MyExt * 0.98f))
				{
					bValid = false;
				}
			}
		}

		else
		{
			bValid = false;
			DesiredT.SetLocation(AimPoint);
		}

		spawnedPart->SetActorTransform(DesiredT);
		spawnedPart->SetPreviewValid(bValid);
	}
}

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
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);
}

void APlayerChar::MoveForward(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}

void APlayerChar::FindObject()
{
	FHitResult HitResult;

	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	if (!isBuilding)
	{
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

			if (Stamina >= 5.0f)
			{
				if (HitResource)
				{
					const FString hitName = HitResource->resourceName;
					const int resourceValue = HitResource->resourceAmount;

					HitResource->totalResource = HitResource->totalResource - resourceValue;

					if (HitResource->totalResource >= resourceValue)
					{
						GiveResource(resourceValue, hitName);

						matsCollected = matsCollected + resourceValue;
						if (objWidget) objWidget->UpdatematOBJ(matsCollected);

						UGameplayStatics::SpawnDecalAtLocation(
							GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f),
							HitResult.Location, FRotator(-90, 0, 0), 2.0f
						);

						SetStamina(-5.0f);
					}
					else
					{
						HitResource->Destroy();
					}
				}
			}
		}
	}
	else
	{
		isBuilding = false;
		objectsBuilt = objectsBuilt + 1.0f;
		if (objWidget) objWidget->UpdatebuildObj(objectsBuilt);
	}
}

void APlayerChar::SetHealth(float amount)
{
	Health = FMath::Clamp(Health + amount, 0.0f, 100.0f);
}

void APlayerChar::SetHunger(float amount)
{
	Hunger = FMath::Clamp(Hunger + amount, 0.0f, 100.0f);
}

void APlayerChar::SetStamina(float amount)
{
	Stamina = FMath::Clamp(Stamina + amount, 0.0f, 100.0f);
}

void APlayerChar::DecreaseStats()
{
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}

	SetStamina(10.0f);

	if (Hunger <= 0)
	{
		SetHealth(-3.0f);
	}
}

void APlayerChar::GiveResource(int32 amount, FString resourceType)
{
	if (ResourcesArray.Num() >= 3)
	{
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

void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0])
	{
		if (stoneAmount <= ResourcesArray[1])
		{
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

			if (buildingObject == "Wall")
			{
				BuildingArray[0] = BuildingArray[0] + 1;
			}

			if (buildingObject == "Floor")
			{
				BuildingArray[1] = BuildingArray[1] + 1;
			}

			if (buildingObject == "Ceiling")
			{
				BuildingArray[2] = BuildingArray[2] + 1;
			}
		}
	}
}

void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	isSuccess = false;

	if (isBuilding)
	{
		return;
	}

	if (!BuildingArray.IsValidIndex(buildingID))
	{
		return;
	}

	if (BuildingArray[buildingID] < 1)
	{
		return;
	}

	if (!BuildPartClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector StartLocation = PlayerCamComp->GetComponentLocation();
	const FVector EndLocation = StartLocation + (PlayerCamComp->GetForwardVector() * 400.0f);
	const FRotator SpawnRot(0.f, 0.f, 0.f);

	ABuildingPart* NewPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, SpawnRot, SpawnParams);
	if (!NewPart)
	{
		return;
	}

	spawnedPart = NewPart;
	isBuilding = true;
	BuildingArray[buildingID] -= 1;

	isSuccess = true;
}

void APlayerChar::RotateBuilding()
{
	if (isBuilding && spawnedPart)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}
