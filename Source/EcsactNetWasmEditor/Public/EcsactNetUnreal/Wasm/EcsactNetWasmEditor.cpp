#include "EcsactNetWasmEditor.h"
#include "EcsactNetEditor/Public/EcsactNetEditor/EcsactNetEditor.h"
#include "EcsactNetEditor/EcsactNetHttpClient.h"
#include "EcsactNetWasm/Public/EcsactNetUnreal/Wasm/EcsactNetWasmSettings.h"
#include "EcsactNetWasm/Public/EcsactNetUnreal/Wasm/EcsactNetWasm.h"
#include "UObject/NoExportTypes.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "ISettingsContainer.h"
#include "Modules/ModuleManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

#define LOCTEXT_NAMESPACE "FEcsactNetWasmEditorModule"

static auto ReadFileAsBase64(const FString& FilePath) -> FString {
	auto file_contents = TArray<uint8>{};
	if(!FFileHelper::LoadFileToArray(file_contents, *FilePath)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to read file: %s"), *FilePath);
		return {};
	}

	return FBase64::Encode(file_contents);
}

auto FEcsactNetWasmEditorModule::StartupModule() -> void {
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
	auto& ecsact_net_editor = FEcsactNetEditorModule::Get();
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	settings_section->OnModified().BindRaw(
		this,
		&FEcsactNetWasmEditorModule::OnSettingsModified
	);

	ecsact_net_editor.AddEcsactNetToolsMenuExtension(
		FMenuExtensionDelegate::CreateRaw(
			this,
			&FEcsactNetWasmEditorModule::AddMenuEntry
		)
	);
}

auto FEcsactNetWasmEditorModule::UploadSystemImpls() -> void {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	auto client = FEcsactNetEditorModule::Get().GetHttpClient();
	auto requests = TArray<FSystemImplsReplaceRequest>{};

	for(auto wasm_file : settings->SystemImplWasmFiles) {
		auto file_path = wasm_file.FilePath;
		if(FPaths::IsRelative(wasm_file.FilePath)) {
			file_path = FPaths::Combine(FPaths::ProjectDir(), file_path);
		}

		auto req = FSystemImplsReplaceRequest{};
		req.fileContentsType = "SYSTEM_IMPL_WASM_BASE64";
		req.fileContents = ReadFileAsBase64(file_path);

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

auto FEcsactNetWasmEditorModule::AddMenuEntry( //
	class FMenuBuilder& MenuBuilder
) -> void {
	MenuBuilder.AddMenuEntry(
		LOCTEXT("EcsactNetUploadWasm", "Upload System Impls"),
		LOCTEXT("EcsactNetUploadWasm", "Manually trigger a wasm system upload"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this] { UploadSystemImpls(); }))
	);
}

auto FEcsactNetWasmEditorModule::OnSettingsModified() -> bool {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	UE_LOG(LogTemp, Warning, TEXT("TODO: watch for wasm file changes"));
	// UploadSystemImpls();

	return true;
}

auto FEcsactNetWasmEditorModule::ShutdownModule() -> void {
}

IMPLEMENT_MODULE(FEcsactNetWasmEditorModule, EcsactNetWasmEditor)
