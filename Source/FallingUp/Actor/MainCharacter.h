// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MainCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AMainCharacter : public APawn
{
	GENERATED_BODY()
	
public:
	AMainCharacter();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


private:
	void UpdateView();

	UFUNCTION()
	void OnBodySleep(UPrimitiveComponent* WakingComponent, FName BoneName);
	UFUNCTION()
	void OnBodyWake(UPrimitiveComponent* WakingComponent, FName BoneName);

	void GravityControlTick();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Body, meta=(AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CameraComponent, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Locomotion, meta=(AllowPrivateAccess = "true"))
	float LocomotionSpeed = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=GravityControl, meta=(AllowPrivateAccess = "true", DisplayName="Angle Threshold"))
	float GravityAngleThreshold = 50.f;

	FQuat ViewOrientation;
	FVector GravityDirection;
	bool bIsBodyWake = true;
};

