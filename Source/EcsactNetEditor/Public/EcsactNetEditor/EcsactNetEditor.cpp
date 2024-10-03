#include "EcsactNetEditor.h"
#include "HAL/PlatformFileManager.h"
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
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EcsactNetHttpClient.h"

#define LOCTEXT_NAMESPACE "FEcsactNetEditorModule"

FEcsactNetEditorModule::FEcsactNetEditorModule() {
	HttpClient = nullptr;
}

FEcsactNetEditorModule::~FEcsactNetEditorModule() {
}

auto FEcsactNetEditorModule::StartupModule() -> void {
	HttpClient = NewObject<UEcsactNetHttpClient>();
}

auto FEcsactNetEditorModule::AddMenuEntry(FMenuBuilder& MenuBuilder) -> void {
}

auto FEcsactNetEditorModule::ShutdownModule() -> void {
}

auto FEcsactNetEditorModule::OnSettingsModified() -> bool {
	return true;
}

auto FEcsactNetEditorModule::Get() -> FEcsactNetEditorModule& {
	return FModuleManager::Get().GetModuleChecked<FEcsactNetEditorModule>("EcsactNetEditor");
}

auto FEcsactNetEditorModule::Login() -> void {
}

auto FEcsactNetEditorModule::GetAuthToken() -> FString {
	auto auth_json_path = FPaths::Combine(
		FPlatformProcess::UserHomeDir(),
		".config",
		"ecsact-net",
		"auth.json"
	);

	auto auth_json_str = FString{};

	if(!FFileHelper::LoadFileToString(auth_json_str, *auth_json_path)) {
		return {};
	}

	auto auth_json = FEcsactNetAuthJson{};
	auto success = FJsonObjectConverter::UStructToJsonObjectString( //
		auth_json,
		auth_json_str
	);

	if(!success) {
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize auth json"));
		return {};
	}

	return auth_json.id_token;
}

IMPLEMENT_MODULE(FEcsactNetEditorModule, EcsactNet)
