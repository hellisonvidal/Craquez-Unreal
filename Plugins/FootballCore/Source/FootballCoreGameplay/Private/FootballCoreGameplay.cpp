// Copyright 2024 Craquez. All Rights Reserved.

#include "FootballCoreGameplay.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFootballCoreGameplay);

void FFootballCoreGameplayModule::StartupModule()
{
    UE_LOG(LogFootballCoreGameplay, Log, TEXT("FootballCoreGameplay Module Started"));
}

void FFootballCoreGameplayModule::ShutdownModule()
{
    UE_LOG(LogFootballCoreGameplay, Log, TEXT("FootballCoreGameplay Module Shutdown"));
}

IMPLEMENT_MODULE(FFootballCoreGameplayModule, FootballCoreGameplay)