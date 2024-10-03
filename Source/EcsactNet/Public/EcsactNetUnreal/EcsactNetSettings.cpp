#include "EcsactNetSettings.h"
#include "EcsactEditor/Public/EcsactEditor.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "FEcsactNetWasmModule"

UEcsactNetSettings::UEcsactNetSettings() {
	auto& settings_module =
		FModuleManager::GetModuleChecked<ISettingsModule>("Settings");

	auto settings_container = settings_module.GetContainer("Project");

	settings_module.RegisterSettings(
		"Project",
		"Plugins",
		"EcsactNet",
		LOCTEXT("SettingsName", "Ecsact Net"),
		LOCTEXT("SettingsDescription", "Configuration settings for Ecsact Net"),
		GetMutableDefault<UEcsactNetSettings>()
	);
}
