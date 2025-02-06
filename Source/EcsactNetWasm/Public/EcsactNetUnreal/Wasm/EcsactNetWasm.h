#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNetWasm, Log, All);

class FEcsactNetWasmModule : public IModuleInterface {
public:
	auto StartupModule() -> void override;
	auto ShutdownModule() -> void override;
};
