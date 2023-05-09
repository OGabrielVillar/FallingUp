// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFallingUp, All, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFallingUpInput, All, All);

#define BIT(x) 1 << x

#define FU_LOG(string, ...) { UE_LOG(LogFallingUp, Log, TEXT(string), __VA_ARGS__); }
#define FU_LOG_FUNC() RT_LOG("%s()", TEXT(__FUNCTION__))
#define FU_LOG_FUNC_AND(string, ...) RT_LOG("%s(" string ")", TEXT(__FUNCTION__), __VA_ARGS__)

#define FU_SCREEN_LOG(string, ...) if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(string), __VA_ARGS__)); }

#define FU_INPUT_LOG(string, ...) { UE_LOG(LogFallingUpInput, Log, TEXT(string), __VA_ARGS__); }

#define FU_ERROR(string, ...) RT_LOG("ERROR: " string, __VA_ARGS__)

template<typename T>
inline constexpr UE::Math::TQuat<T> QuatDistance(const UE::Math::TQuat<T>& From, const UE::Math::TQuat<T>& To)
{
	return To.Inverse() * From;
}