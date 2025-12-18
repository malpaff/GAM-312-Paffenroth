#include "BuildingPart.h"

ABuildingPart::ABuildingPart()
{
	PrimaryActorTick.bCanEverTick = false;

	PivotArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Pivot Arrow"));
	RootComponent = PivotArrow;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(PivotArrow);

	SP_North = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_North"));
	SP_South = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_South"));
	SP_East = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_East"));
	SP_West = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_West"));
	SP_Top = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_Top"));
	SP_Bottom = CreateDefaultSubobject<UArrowComponent>(TEXT("SP_Bottom"));

	SP_North->SetupAttachment(PivotArrow);
	SP_South->SetupAttachment(PivotArrow);
	SP_East->SetupAttachment(PivotArrow);
	SP_West->SetupAttachment(PivotArrow);
	SP_Top->SetupAttachment(PivotArrow);
	SP_Bottom->SetupAttachment(PivotArrow);

	constexpr float ArrowSize = 0.75f;
	SP_North->ArrowSize = ArrowSize;
	SP_South->ArrowSize = ArrowSize;
	SP_East->ArrowSize = ArrowSize;
	SP_West->ArrowSize = ArrowSize;
	SP_Top->ArrowSize = ArrowSize;
	SP_Bottom->ArrowSize = ArrowSize;

	UpdateSnapPoints();
}

void ABuildingPart::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateSnapPoints();
}

void ABuildingPart::BeginPlay()
{
	Super::BeginPlay();
}

static FVector GetMeshExtentsLocal(const UStaticMeshComponent* MeshComp)
{
	if (!MeshComp || !MeshComp->GetStaticMesh())
	{
		return FVector(50.f, 50.f, 50.f);
	}

	const FBoxSphereBounds LocalBounds = MeshComp->CalcBounds(FTransform::Identity);
	return LocalBounds.BoxExtent; 
}

void ABuildingPart::UpdateSnapPoints()
{
	const FVector Extents = GetMeshExtentsLocal(Mesh);

	const float HalfX = Extents.X;
	const float HalfY = Extents.Y;
	const float HalfZ = Extents.Z;

	// Locations (relative to pivot)
	float EdgeZ = 0.f;

	if (PartType == EBuildingPartType::Floor)
	{
		EdgeZ = +HalfZ;
	}

	if (SP_North)  SP_North->SetRelativeLocation(FVector(0.f, +HalfY, EdgeZ));
	if (SP_South)  SP_South->SetRelativeLocation(FVector(0.f, -HalfY, EdgeZ));
	if (SP_East)   SP_East->SetRelativeLocation(FVector(+HalfX, 0.f, EdgeZ));
	if (SP_West)   SP_West->SetRelativeLocation(FVector(-HalfX, 0.f, EdgeZ));

	if (SP_Top)    SP_Top->SetRelativeLocation(FVector(0.f, 0.f, +HalfZ));
	if (SP_Bottom) SP_Bottom->SetRelativeLocation(FVector(0.f, 0.f, -HalfZ));


	// Rotations
	if (SP_North)  SP_North->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));    // +Y
	if (SP_South)  SP_South->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));   // -Y
	if (SP_East)   SP_East->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));      // +X
	if (SP_West)   SP_West->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));    // -X
	if (SP_Top)    SP_Top->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));     // +Z
	if (SP_Bottom) SP_Bottom->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));   // -Z
}

static UArrowComponent* GetSnapComponent(const ABuildingPart* Part, ESnapPoint Point)
{
	if (!Part) return nullptr;

	switch (Point)
	{
	case ESnapPoint::North:  return Part->SP_North;
	case ESnapPoint::South:  return Part->SP_South;
	case ESnapPoint::East:   return Part->SP_East;
	case ESnapPoint::West:   return Part->SP_West;
	case ESnapPoint::Top:    return Part->SP_Top;
	case ESnapPoint::Bottom: return Part->SP_Bottom;
	default:                 return nullptr;
	}
}

FTransform ABuildingPart::GetSnapTransform(ESnapPoint Point) const
{
	if (UArrowComponent* Comp = GetSnapComponent(this, Point))
	{
		return Comp->GetComponentTransform(); // WORLD transform
	}
	return GetActorTransform();
}

FTransform ABuildingPart::GetSnapRelativeTransform(ESnapPoint Point) const
{
	if (UArrowComponent* Comp = GetSnapComponent(this, Point))
	{
		const FTransform SnapWorld = Comp->GetComponentTransform();
		const FTransform ActorWorld = GetActorTransform();

		FTransform SnapActorLocal = SnapWorld.GetRelativeTransform(ActorWorld);

		SnapActorLocal.SetScale3D(FVector(1.f));

		return SnapActorLocal;
	}

	return FTransform::Identity;
}
TArray<UArrowComponent*> ABuildingPart::GetAllSnapPoints() const
{
	TArray<UArrowComponent*> Points;
	Points.Reserve(6);

	switch (PartType)
	{
	case EBuildingPartType::Floor:
	case EBuildingPartType::Ceiling:
	case EBuildingPartType::Roof:
		if (SP_North) Points.Add(SP_North);
		if (SP_South) Points.Add(SP_South);
		if (SP_East)  Points.Add(SP_East);
		if (SP_West)  Points.Add(SP_West);
		break;

	case EBuildingPartType::Wall:
		if (SP_Top)    Points.Add(SP_Top);
		if (SP_Bottom) Points.Add(SP_Bottom);

		if (SP_North) Points.Add(SP_North);
		if (SP_South) Points.Add(SP_South);
		if (SP_East)  Points.Add(SP_East);
		if (SP_West)  Points.Add(SP_West);
		break;

	default:
		// Fallback
		if (SP_North)  Points.Add(SP_North);
		if (SP_South)  Points.Add(SP_South);
		if (SP_East)   Points.Add(SP_East);
		if (SP_West)   Points.Add(SP_West);
		if (SP_Top)    Points.Add(SP_Top);
		if (SP_Bottom) Points.Add(SP_Bottom);
		break;
	}

	return Points;
}

void ABuildingPart::SetPreviewValid(bool bValid)
{
	if (!Mesh) return;

	if (!PreviewMID)
	{
		PreviewMID = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (PreviewMID)
	{
		PreviewMID->SetVectorParameterValue(TEXT("TintColor"), bValid ? FLinearColor(1, 1, 1, 1) : FLinearColor(1, 0, 0, 1));
	}
}