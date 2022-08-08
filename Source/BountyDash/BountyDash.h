// Copyright Epic Games, Inc. All Rights Reserved.
// Working as pre-compiled header

#pragma once

#include "CoreMinimal.h"

template<typename T>
T* GetCustomGameMode(UWorld* WorldContext)
{
	return Cast<T>(WorldContext->GetAuthGameMode());
}