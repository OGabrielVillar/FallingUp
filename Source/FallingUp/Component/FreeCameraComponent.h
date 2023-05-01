// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "FreeCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class FALLINGUP_API UFreeCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
	void SetRotation(const FQuat& Rotation);
};
