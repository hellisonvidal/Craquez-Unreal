// Copyright 2024 Craquez. All Rights Reserved.

#include "Craquez.h"
#include "Modules/ModuleManager.h"

void FCraquezModule::StartupModule()
{
    // Register game-specific systems here
    UE_LOG(LogTemp, Log, TEXT("Craquez Module Started"));
}

void FCraquezModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("Craquez Module Shutdown"));
}

IMPLEMENT_MODULE(FCraquezModule, Craquez)