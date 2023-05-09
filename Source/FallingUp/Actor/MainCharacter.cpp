// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "../FallingUp.h"

AMainCharacter::AMainCharacter()
	: ViewOrientation(ForceInit)
	, GravityDirection(0.f,0.f,-1.0)
{
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(25.f, 25.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;
	
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-3.f, 0.f, -13.f)); // Position the camera
	CameraComponent->SetupAttachment(CapsuleComponent);
	CameraComponent->bConstrainAspectRatio = false;
}

void AMainCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		ViewOrientation = Controller->GetControlRotation().Quaternion();

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	CapsuleComponent->OnComponentSleep.AddDynamic(this, &AMainCharacter::OnBodySleep);
	CapsuleComponent->OnComponentWake.AddDynamic(this, &AMainCharacter::OnBodyWake);
}

void AMainCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bIsBodyWake)
		GravityControlTick();

	CapsuleComponent->SetPhysicsLinearVelocity(GravityDirection * 10.f, true);
}

//////////////////////////////////////////////////////////////////////////// Input

void AMainCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainCharacter::Look);
	}
}


void AMainCharacter::Move(const FInputActionValue& Value)
{
	if (CapsuleComponent == nullptr)
		return;

	FVector2d DirectionInput = Value.Get<FVector2d>();
	DirectionInput.Normalize();

	FVector Direction = ViewOrientation.RotateVector(FVector(DirectionInput, 0.f));

	CapsuleComponent->SetPhysicsLinearVelocity(Direction * LocomotionSpeed, true);

	//GetWorld()->LineTraceMultiByChannel()
}

constexpr float ViewRotationSpeed = 0.04f;

void AMainCharacter::Look(const FInputActionValue& Value)
{
	if (Value[1])
	{
		ViewOrientation *= FQuat({0.f,-1.f,0.f}, Value[1] * ViewRotationSpeed);
	}

	if (Value[0])
	{
		FVector YawAxis = ViewOrientation.UnrotateVector(-GravityDirection);
		ViewOrientation *= FQuat(YawAxis, Value[0] * ViewRotationSpeed);
	}

	UpdateView();
}

void AMainCharacter::UpdateView()
{
	if (Controller == nullptr)
		return;

	Controller->SetControlRotation(ViewOrientation.Rotator());
	CameraComponent->SetWorldRotation(ViewOrientation);
}

void AMainCharacter::OnBodySleep(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	FU_SCREEN_LOG("sleep");
	bIsBodyWake = false;
}

void AMainCharacter::OnBodyWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	FU_SCREEN_LOG("wake");
	bIsBodyWake = true;
}

void AMainCharacter::GravityControlTick()
{	
	FHitResult HitResult;

	FVector TraceA = GetActorLocation();
	FVector TraceB = TraceA + (1.4f * GravityDirection * CapsuleComponent->GetScaledCapsuleHalfHeight());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Cast<AActor>(this));
	
	GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		TraceA,
		TraceB,
		ECollisionChannel::ECC_WorldDynamic,
		QueryParams
	);
	
	if (HitResult.bBlockingHit)
	{
		float Dot = HitResult.Normal.Dot(GravityDirection);

		bool DirectionsAreDifferent = Dot != -1.f;

		if (DirectionsAreDifferent)
		{
			bool DifferencePassThreshold = Dot <= -FMath::Cos(FMath::DegreesToRadians(GravityAngleThreshold));

			if (DifferencePassThreshold)
			{
				FVector NewGravityDirection = -HitResult.Normal;
				FQuat OrientationDifference = FQuat::FindBetweenNormals(GravityDirection, NewGravityDirection);
				
				GravityDirection = NewGravityDirection;
				
				ViewOrientation = OrientationDifference * ViewOrientation;
				UpdateView();

			}
			auto ThresholdCos = FString::SanitizeFloat(-FMath::Cos(FMath::DegreesToRadians(GravityAngleThreshold)));
			FU_SCREEN_LOG("ThresholdCos: %s", *ThresholdCos);
			auto LogMsg = FString::SanitizeFloat(Dot);
			FU_SCREEN_LOG("Dot: %s", *LogMsg);
		}
	}
}
