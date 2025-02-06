#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNetWasmEditor, Log, All);

class FEcsactNetWasmEditorModule : public IModuleInterface {
	auto OnSettingsModified() -> bool;
	auto UploadSystemImpls() -> void;

public:
	auto StartupModule() -> void override;
	auto AddMenuEntry( //
		class FMenuBuilder& MenuBuilder
	) -> void;
	auto ShutdownModule() -> void override;
};
