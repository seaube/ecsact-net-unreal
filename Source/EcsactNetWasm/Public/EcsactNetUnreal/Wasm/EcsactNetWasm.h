#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNet, Log, All);

class FEcsactNetWasmModule : public IModuleInterface {
	auto OnSettingsModified() -> bool;
	auto UploadSystemImpls() -> void;
	auto AddMenuEntry(class FMenuBuilder&) -> void;

public:
	auto StartupModule() -> void override;
	auto ShutdownModule() -> void override;
};
