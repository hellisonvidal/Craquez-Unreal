// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFootballCoreGameplayModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FFootballCoreGameplayModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FFootballCoreGameplayModule>("FootballCoreGameplay");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("FootballCoreGameplay");
    }
};

DECLARE_LOG_CATEGORY_EXTERN(LogFootballCoreGameplay, Log, All);