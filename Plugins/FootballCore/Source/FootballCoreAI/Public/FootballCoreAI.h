// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFootballCoreAIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FFootballCoreAIModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FFootballCoreAIModule>("FootballCoreAI");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("FootballCoreAI");
    }
};

DECLARE_LOG_CATEGORY_EXTERN(LogFootballCoreAI, Log, All);