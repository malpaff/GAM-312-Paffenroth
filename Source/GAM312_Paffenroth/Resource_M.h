#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Resource_M.generated.h"

UCLASS()
class GAM312_PAFFENROTH_API AResource_M : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AResource_M();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Name of the resource
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		FString resourceName = "Wood";

	// Amount of resource given to the player on interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		int resourceAmount = 5;

	// Amount of resource that is available before depleting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
		int totalResource = 100;

	// Displays the resource's name in front of the mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
		UTextRenderComponent* ResourceNameTxt;

	// Currently, the cube that represents the resource
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
		UStaticMeshComponent* Mesh;
};
