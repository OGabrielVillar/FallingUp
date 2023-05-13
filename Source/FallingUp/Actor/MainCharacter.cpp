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
	CapsuleComponent->OnComponentHit.AddDynamic(this, &AMainCharacter::OnBodyHit);
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
	FU_SCREEN_LOG("AMainCharacter::OnBodySleep()");
	bIsBodyWake = false;
}

void AMainCharacter::OnBodyWake(UPrimitiveComponent* WakingComponent, FName BoneName)
{
	FU_SCREEN_LOG("AMainCharacter::OnBodyWake()");
	bIsBodyWake = true;
}

void AMainCharacter::OnBodyHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	FU_SCREEN_LOG("AMainCharacter::GravityControlTick()");
}

void AMainCharacter::GravityControlTick()
{	
	FHitResult HitResult;

	FVector TraceA = GetActorLocation();
	FVector TraceB = TraceA + (1.4f * GravityDirection * CapsuleComponent->GetScaledCapsuleHalfHeight());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Cast<AActor>(this));
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		TraceA,
		TraceB,
		ECollisionChannel::ECC_WorldDynamic,
		QueryParams
	);
	
	if (HitResult.bBlockingHit)
	{
		FVector Normal;

		Normal = HitResult.ImpactNormal;

		// WORKING ON: Getting tiangle vertices's normal and position.
		UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(HitResult.GetComponent());
		if (StaticMeshComp != nullptr)
		{
			//FU_SCREEN_LOG("valid StaticMeshComp: %s", *StaticMeshComp->GetName());
			UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
			if (StaticMesh != nullptr)
			{
				//FU_SCREEN_LOG("valid StaticMesh: %s | CPUAccess [%i]", *StaticMesh->GetFullName(), (uint32)StaticMesh->bAllowCPUAccess);

				uint32 FaceIndex = HitResult.FaceIndex;

				FStaticMeshVertexBuffers* VertexBuffers = &StaticMesh->GetRenderData()->LODResources[0].VertexBuffers;
				FStaticMeshVertexBuffer* StaticMeshVertexBuffer = &VertexBuffers->StaticMeshVertexBuffer;
				FPositionVertexBuffer* PositionVertexBuffer = &VertexBuffers->PositionVertexBuffer;
				
				FIndexArrayView IndexBuffer = StaticMesh->GetRenderData()->LODResources[0].IndexBuffer.GetArrayView();
				
				FVector VertexPositions[3];
				FVector VertexNormals[3];
				
				//FU_SCREEN_LOG("IndexBufferNum %i", IndexBuffer.Num());

				uint32 index0 = IndexBuffer[FaceIndex * 3 + 0];
				VertexPositions[0] = FVector(PositionVertexBuffer->VertexPosition(index0));
				VertexNormals[0] = FVector(StaticMeshVertexBuffer->VertexTangentZ(index0));
				
				uint32 index1 = IndexBuffer[FaceIndex * 3 + 1];
				VertexPositions[1] = FVector(PositionVertexBuffer->VertexPosition(index1));
				VertexNormals[1] = FVector(StaticMeshVertexBuffer->VertexTangentZ(index1));
				
				uint32 index2 = IndexBuffer[FaceIndex * 3 + 2];
				VertexPositions[2] = FVector(PositionVertexBuffer->VertexPosition(index2));
				VertexNormals[2] = FVector(StaticMeshVertexBuffer->VertexTangentZ(index2));
				
				//FU_SCREEN_LOG("ImpactPoint: %s.", *HitResult.ImpactPoint.ToString());
				
				// Transform Hitpoint Into Local Space
				FVector LocalHitPoint = StaticMeshComp->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);
				//FU_SCREEN_LOG("LocalHitPoint: %s.", *LocalHitPoint.ToString());
				
				// Store normal vector based on barycentric interpolation
				FVector BaryNormal;
				
				// Get barycentric coordinates
				FVector b = FMath::ComputeBaryCentric2D(LocalHitPoint, VertexPositions[0], VertexPositions[1], VertexPositions[2]);
				
				BaryNormal.X = b.X * VertexNormals[0].X + b.Y * VertexNormals[1].X + b.Z * VertexNormals[2].X;
				BaryNormal.Y = b.X * VertexNormals[0].Y + b.Y * VertexNormals[1].Y + b.Z * VertexNormals[2].Y;
				BaryNormal.Z = b.X * VertexNormals[0].Z + b.Y * VertexNormals[1].Z + b.Z * VertexNormals[2].Z;
				
				//FU_SCREEN_LOG("BaryNormal(PreWorldRotation): %s.", *BaryNormal.ToString());
				
				// To WorldSpace Orientation
				BaryNormal = StaticMeshComp->GetComponentTransform().GetRotation().RotateVector(BaryNormal);
				
				// Normalize
				BaryNormal.Normalize();
				
				//FU_SCREEN_LOG("Pre Normal Correction: %s.", *Normal.ToString());
				//FU_SCREEN_LOG("Post Normal Correction: %s.", *BaryNormal.ToString());
				
				Normal = BaryNormal;
			}
		}

		float Dot = Normal.Dot(GravityDirection);

		bool DirectionsAreDifferent = Dot != -1.f;

		if (DirectionsAreDifferent)
		{
			bool DifferencePassThreshold = Dot <= -FMath::Cos(FMath::DegreesToRadians(GravityAngleThreshold));

			if (DifferencePassThreshold)
			{
				FVector NewGravityDirection = -Normal;
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
