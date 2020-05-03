// Copyright Epic Games, Inc. All Rights Reserved.

#include "Flocking.h"
#include "Modules/ModuleManager.h"

#include "Misc/Paths.h"
#include "ShaderCore.h"


void FFlocking::StartupModule() {
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/Project", ShaderDirectory);
}

void FFlocking::ShutdownModule() {
	ResetAllShaderSourceDirectoryMappings();
}
IMPLEMENT_PRIMARY_GAME_MODULE(FFlocking, Flocking, "Flocking");


