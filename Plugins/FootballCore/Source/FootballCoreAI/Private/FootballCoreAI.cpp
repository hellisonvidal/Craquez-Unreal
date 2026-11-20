// Copyright 2024 Craquez. All Rights Reserved.

#include "FootballCoreAI.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFootballCoreAI);

void FFootballCoreAIModule::StartupModule()
{
    UE_LOG(LogFootballCoreAI, Log, TEXT("FootballCoreAI Module Started"));
}

void FFootballCoreAIModule::ShutdownModule()
{
    UE_LOG(LogFootballCoreAI, Log, TEXT("FootballCoreAI Module Shutdown"));
}

IMPLEMENT_MODULE(FFootballCoreAIModule, FootballCoreAI)