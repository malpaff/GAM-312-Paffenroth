// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveWidget.generated.h"

/**
 * 
 */
UCLASS()
class GAM312_PAFFENROTH_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	// Function to update UI when materials are collected
	UFUNCTION(BlueprintImplementableEvent)
		void UpdatematOBJ(float matsCollected);

	// Function to update UI when building parts are placed
	UFUNCTION(BlueprintImplementableEvent)
		void UpdatebuildObj(float objectsBuilt);

};
