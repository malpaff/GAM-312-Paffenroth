#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "BuildingPart.generated.h"

UENUM(BlueprintType)
enum class EBuildingPartType : uint8
{
	Floor  UMETA(DisplayName = "Floor"),
	Wall   UMETA(DisplayName = "Wall"),
	Roof   UMETA(DisplayName = "Roof"),
	Ceiling UMETA(DisplayName = "Ceiling")
};

UENUM(BlueprintType)
enum class ESnapPoint : uint8
{
	North,
	South,
	East,
	West,
	Top,
	Bottom
};

UCLASS()
class GAM312_PAFFENROTH_API ABuildingPart : public AActor
{
	GENERATED_BODY()

public:
	ABuildingPart();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	UArrowComponent* PivotArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_North;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_South;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_East;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_West;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_Top;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snapping")
	UArrowComponent* SP_Bottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	EBuildingPartType PartType = EBuildingPartType::Floor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	FVector PartSize = FVector(200.f, 200.f, 10.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMaterialInstanceDynamic* PreviewMID = nullptr;

	UFUNCTION()
	void SetPreviewValid(bool bValid);

	UFUNCTION(BlueprintCallable, Category = "Snapping")
	FTransform GetSnapTransform(ESnapPoint Point) const;

	UFUNCTION(BlueprintCallable, Category = "Snapping")
	TArray<UArrowComponent*> GetAllSnapPoints() const;

	FTransform GetSnapRelativeTransform(ESnapPoint Point) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void UpdateSnapPoints();
};