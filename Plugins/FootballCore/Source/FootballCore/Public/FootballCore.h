// Copyright 2024 Craquez. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "FootballCoreTypes.h"

class FFootballCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FFootballCoreModule& Get() { return FModuleManager::LoadModuleChecked<FFootballCoreModule>("FootballCore"); }

    static bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("FootballCore"); }
};

// Log category
DECLARE_LOG_CATEGORY_EXTERN(LogFootballCore, Log, All);