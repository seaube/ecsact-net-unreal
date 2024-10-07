#include "EcsactNetWasm.h"
#include "UObject/NoExportTypes.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "ISettingsContainer.h"
#include "Modules/ModuleManager.h"
#include "EcsactNetWasmSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "EcsactNetEditor/EcsactNetEditor.h"
#include "EcsactNetEditor/EcsactNetHttpClient.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#define LOCTEXT_NAMESPACE "FEcsactNetWasmModule"

static auto ReadFileAsBase64(const FString& FilePath) -> FString {
	auto file_contents = TArray<uint8>{};
	if(!FFileHelper::LoadFileToArray(file_contents, *FilePath)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
		return {};
	}

	return FBase64::Encode(file_contents);
}

auto FEcsactNetWasmModule::StartupModule() -> void {
	auto& settings_module =
		FModuleManager::GetModuleChecked<ISettingsModule>("Settings");
	auto settings_container = settings_module.GetContainer("Project");
	auto settings_section = settings_module.RegisterSettings(
		"Project",
		"Plugins",
		"EcsactNetWasm",
		LOCTEXT("SettingsName", "Ecsact Net Wasm Settings"),
		LOCTEXT(
			"SettingsDescription",
			"Configuration settings for Ecsact Net Wasm System Implementations"
		),
		GetMutableDefault<UEcsactNetWasmSettings>()
	);
	check(settings_section.IsValid());

	settings_section->OnModified().BindRaw(
		this,
		&FEcsactNetWasmModule::OnSettingsModified
	);

	auto& ecsact_net_editor = FEcsactNetEditorModule::Get();

	ecsact_net_editor.AddEcsactNetToolsMenuExtension(
		FMenuExtensionDelegate::CreateRaw(this, &FEcsactNetWasmModule::AddMenuEntry)
	);
}

auto FEcsactNetWasmModule::AddMenuEntry( //
	class FMenuBuilder& MenuBuilder
) -> void {
	MenuBuilder.AddMenuEntry(
		LOCTEXT("EcsactNetUploadWasm", "Upload System Impls"),
		LOCTEXT("EcsactNetUploadWasm", "Manually trigger a wasm system upload"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { UploadSystemImpls(); }))
	);
}

auto FEcsactNetWasmModule::ShutdownModule() -> void {
}

auto FEcsactNetWasmModule::UploadSystemImpls() -> void {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	auto client = FEcsactNetEditorModule::Get().GetHttpClient();
	auto requests = TArray<FSystemImplsReplaceRequest>{};

	for(auto wasm_file : settings->SystemImplWasmFiles) {
		auto req = FSystemImplsReplaceRequest{};
		req.fileContentsType = "SYSTEM_IMPL_WASM_BASE64";
		req.fileContents = ReadFileAsBase64(wasm_file.FilePath);

		if(req.fileContents.IsEmpty()) {
			return;
		}

		auto req_json = FString{};

		if(!FJsonObjectConverter::UStructToJsonObjectString(req, req_json)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize request payload"));
			return;
		}

		requests.Add(req);
	}

	client->UploadSystemImpls(
		std::move(requests),
		TDelegate<void(TArray<FSystemImplsReplaceResponse>)>::CreateLambda(
			[](auto response) {
				for(auto item : response) {
					if(item.status == "SIS_OK") {
						UE_LOG(LogTemp, Log, TEXT("Successfully uploaded wasm!"));
						UE_LOG(LogTemp, Log, TEXT("Uploaded System Names:"));
						for(auto name : item.systemNames) {
							UE_LOG(LogTemp, Log, TEXT("    - %s"), *name);
						}
					} else {
						UE_LOG(
							LogTemp,
							Log,
							TEXT("Failed to upload wasm. Status is %s"),
							*item.status
						);
					}
				}
			}
		)
	);
}

auto FEcsactNetWasmModule::OnSettingsModified() -> bool {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	UE_LOG(LogTemp, Warning, TEXT("TODO: watch for wasm file changes"));
	// UploadSystemImpls();

	return true;
}

IMPLEMENT_MODULE(FEcsactNetWasmModule, EcsactNet)
