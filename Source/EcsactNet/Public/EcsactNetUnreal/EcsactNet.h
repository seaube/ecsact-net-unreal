#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNet, Log, All);

class FEcsactNetModule : public IModuleInterface {
public:
	auto StartupModule() -> void override;
	auto ShutdownModule() -> void override;
};
