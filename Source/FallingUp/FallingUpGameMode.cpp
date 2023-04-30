// Copyright Epic Games, Inc. All Rights Reserved.

#include "FallingUpGameMode.h"
#include "FallingUpCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFallingUpGameMode::AFallingUpGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
