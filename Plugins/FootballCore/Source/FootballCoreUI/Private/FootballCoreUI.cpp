// Copyright 2024 Craquez. All Rights Reserved.

#include "FootballCoreUI.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFootballCoreUI);

void FFootballCoreUIModule::StartupModule()
{
    UE_LOG(LogFootballCoreUI, Log, TEXT("FootballCoreUI Module Started"));
}

void FFootballCoreUIModule::ShutdownModule()
{
    UE_LOG(LogFootballCoreUI, Log, TEXT("FootballCoreUI Module Shutdown"));
}

IMPLEMENT_MODULE(FFootballCoreUIModule, FootballCoreUI)