// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FFootballCoreUIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FFootballCoreUIModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FFootballCoreUIModule>("FootballCoreUI");
    }

    static bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("FootballCoreUI"); }
};

DECLARE_LOG_CATEGORY_EXTERN(LogFootballCoreUI, Log, All);