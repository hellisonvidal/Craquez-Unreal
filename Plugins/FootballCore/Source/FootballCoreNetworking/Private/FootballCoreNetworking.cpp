// Copyright 2024 Craquez. All Rights Reserved.

#include "FootballCoreNetworking.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogFootballCoreNetworking);

void FFootballCoreNetworkingModule::StartupModule()
{
    UE_LOG(LogFootballCoreNetworking, Log, TEXT("FootballCoreNetworking Module Started"));
}

void FFootballCoreNetworkingModule::ShutdownModule()
{
    UE_LOG(LogFootballCoreNetworking, Log, TEXT("FootballCoreNetworking Module Shutdown"));
}

IMPLEMENT_MODULE(FFootballCoreNetworkingModule, FootballCoreNetworking)