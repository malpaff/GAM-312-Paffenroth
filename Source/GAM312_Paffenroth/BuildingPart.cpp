#include "BuildingPart.h"

// Sets default values
ABuildingPart::ABuildingPart()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach components
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	PivotArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Pivot Arrow"));

	// Set the pivot as the root and attach the mesh to it
	RootComponent = PivotArrow;
	Mesh->SetupAttachment(PivotArrow);

}

// Called when the game starts or when spawned
void ABuildingPart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildingPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

