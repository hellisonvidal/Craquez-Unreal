// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFootballCoreNetworkingModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FFootballCoreNetworkingModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FFootballCoreNetworkingModule>("FootballCoreNetworking");
    }

    static bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("FootballCoreNetworking"); }
};

DECLARE_LOG_CATEGORY_EXTERN(LogFootballCoreNetworking, Log, All);