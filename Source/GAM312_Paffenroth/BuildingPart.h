#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "BuildingPart.generated.h"

UCLASS()
class GAM312_PAFFENROTH_API ABuildingPart : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildingPart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Static mesh that represents the building part
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* Mesh;

	// Arrow component used as the pivot
	UPROPERTY(EditAnywhere)
		UArrowComponent* PivotArrow;

};
