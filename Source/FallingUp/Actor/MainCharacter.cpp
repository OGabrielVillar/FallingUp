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
	, GravityOrientation(FVector(0.f,0.f,-1.0).ToOrientationQuat())
{
	bAllowTickBeforeBeginPlay = false;
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(10.f, 10.0f);
		
	// Create a CameraComponent	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetRelativeLocation(FVector(-3.f, 0.f, -13.f)); // Position the camera
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bConstrainAspectRatio = false;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(GetCapsuleComponent());
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
}

void AMainCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

    // Set the character's gravity scale to zero
    GetCharacterMovement()->GravityScale = 0.0f;

    // Enable custom gravity
    GetCharacterMovement()->SetPlaneConstraintEnabled(true);

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		ViewOrientation = Controller->GetControlRotation().Quaternion();

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//GetCapsuleComponent()->OnInputTouchBegin.AddDynamic(this, &AMainCharacter::LogNothing0);
	//GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMainCharacter::LogNothing1);
	//GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMainCharacter::LogNothing2);

}

void AMainCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector GravityVector = GravityOrientation.Vector();

	//LaunchCharacter(GravityVector * DeltaSeconds * 1000.f, false, false);

    GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Falling;
    GetCharacterMovement()->Velocity += GravityVector * 10000.f * DeltaSeconds;
    //GetCharacterMovement()->AddForce(GravityVector * 10000.f);

    GetCharacterMovement()->SetPlaneConstraintNormal(-GravityVector);

	float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	FVector TraceA = GetActorLocation();
	FVector TraceB = TraceA + (1.4f * GravityOrientation.Vector() * GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	
	FU_SCREEN_LOG("%i, %i", (int)GetCharacterMovement()->IsActive(), (int)GetCharacterMovement()->HasValidData());

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Cast<AActor>(this));

	GetWorld()->SweepSingleByChannel(
		HitResult,
		TraceA,
		TraceB,
		FQuat(ForceInit),
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(CapsuleRadius),
		QueryParams
	);

	if (HitResult.bBlockingHit)
	{
		TraceB = HitResult.Location;
		if (HitResult.Normal.Dot(GravityOrientation.Vector()))
		{

		};

		const auto& Result = HitResult.Normal.Dot(GravityOrientation.Vector());
		auto LogMsg = FString::SanitizeFloat(Result);
		FU_SCREEN_LOG("%s", *LogMsg);
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void AMainCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainCharacter::Look);
	}
}


void AMainCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		FVector2d DirectionInput = Value.Get<FVector2d>();
		DirectionInput.Normalize();

		FVector Direction = ViewOrientation.RotateVector(FVector(DirectionInput, 0.f));

		double mSpeedScale = 15.f;

		// add movement 
		AddMovementInput(Direction, mSpeedScale);
	}
}

void AMainCharacter::Look(const FInputActionValue& Value)
{
	FVector2D ViewRotationSpeed = {0.04f, 0.04f};

	if (Value[1])
	{
		const FVector TiltAxis = { 0.f,-1.f,0.f };
		ViewOrientation *= FQuat(TiltAxis, ViewRotationSpeed.Y * Value[1]);
	}
	if (Value[0])
	{
		const FVector PanAxis = ViewOrientation.Inverse().RotateVector({ 0.f,0.f,1.f });
		ViewOrientation *= FQuat(PanAxis, ViewRotationSpeed.X * Value[0]);
	}

	if (Controller != nullptr)
	{
		Controller->SetControlRotation(ViewOrientation.Rotator());
		FirstPersonCamera->SetWorldRotation(ViewOrientation);
	}

}
