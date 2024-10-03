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
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "EcsactNetWasmUpload.h"

#define LOCTEXT_NAMESPACE "FEcsactNetWasmModule"

static auto ReadFileAsBase64(const FString& FilePath) -> FString {
	// Read file contents into a byte array
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

	auto& level_editor_module =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FExtender> menu_extender = MakeShareable(new FExtender());
	menu_extender->AddMenuExtension(
		"Tools",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FEcsactNetWasmModule::AddMenuEntry)
	);

	level_editor_module.GetMenuExtensibilityManager()->AddExtender(menu_extender);
}

auto FEcsactNetWasmModule::AddMenuEntry(FMenuBuilder& MenuBuilder) -> void {
	MenuBuilder.BeginSection(
		"EcsactNetTools",
		LOCTEXT("EcsactNetToolsSectionTitle", "Ecsact Net")
	);
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("EcsactNetUploadWasm", "Upload System Impls"),
			LOCTEXT("EcsactNetUploadWasm", "Manually trigger a wasm system upload"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this] { UploadSystemImpls(); }))
		);
	}
	MenuBuilder.EndSection();
}

auto FEcsactNetWasmModule::ShutdownModule() -> void {
}

auto FEcsactNetWasmModule::UploadSystemImpls() -> void {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();
	if(settings->SystemImplWasmFiles.IsEmpty()) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("No wasm system impls files are configured. Please review your "
					 "Ecsact Net settings.")
		);
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Attempting to upload wasm system impl files!")
	);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> http_request =
		FHttpModule::Get().CreateRequest();

	auto req_json_stream = FString{"["};

	for(auto wasm_file : settings->SystemImplWasmFiles) {
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Encoding wasm system impl file: %s"),
			*wasm_file.FilePath
		);

		auto req = FSystemImplsReplaceRequest{};
		req.file_contents_type = "SYSTEM_IMPL_WASM_BASE64";
		req.file_contents = ReadFileAsBase64(wasm_file.FilePath);

		if(req.file_contents.IsEmpty()) {
			return;
		}

		auto req_json = FString{};

		if(!FJsonObjectConverter::UStructToJsonObjectString(req, req_json)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize request payload"));
			return;
		}

		req_json_stream += req_json + ",\n";
	}
	req_json_stream.RemoveFromEnd(",\n");
	req_json_stream += "\n]";

	// TODO: make this api url configurable
	http_request->SetURL(
		"http://172.23.68.63:51051/v1/project/system-impls/replace"
	);
	http_request->SetVerb("POST");

	// TODO: get project id from settings
	http_request->SetHeader("project_id", "66d0c11af61ac64dc6063793");
	// TODO: get auth token from REST request
	http_request->SetHeader(
		"ecsact-net-token",
		// NOTE: this is zeke's token he added on 2024-10-02
		//       can be found in ~/.config/ecsact-net/auth.json (key is `id_token`)
		//       available after running `ecsact_net login`
		"eyJhbGciOiJSUzI1NiIsImtpZCI6IjZjYzNmY2I2NDAzMjc2MGVlYjljMjZmNzdkNDA3YTY5NG"
		"M1MmIwZTMiLCJ0eXAiOiJKV1QifQ."
		"eyJuYW1lIjoiRXpla2llbCBXYXJyZW4iLCJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2"
		"dsZS5jb20vc2VhdWJlIiwiYXVkIjoic2VhdWJlIiwiYXV0aF90aW1lIjoxNzI0OTU1NzI0LCJ1"
		"c2VyX2lkIjoib2ljWVZOaW5PN1MwUDV1UzVHbmpJSTZtYktYMiIsInN1YiI6Im9pY1lWTmluTz"
		"dTMFA1dVM1R25qSUk2bWJLWDIiLCJpYXQiOjE3Mjc5MTM1NjcsImV4cCI6MTcyNzkxNzE2Nywi"
		"ZW1haWwiOiJlemVraWVsQHNlYXViZS5jb20iLCJlbWFpbF92ZXJpZmllZCI6dHJ1ZSwiZmlyZW"
		"Jhc2UiOnsiaWRlbnRpdGllcyI6eyJhcHBsZS5jb20iOlsiMDAwOTUyLmMxNjdlOGQwZjc0NzQ4"
		"YzU4Yjg4NzY2NDlkNjZkMjI5LjA0NTkiXSwiZ29vZ2xlLmNvbSI6WyIxMTI2MzI1NDE4NTA0MD"
		"Y1MTAwMTQiXSwiZW1haWwiOlsiZXpla2llbEBzZWF1YmUuY29tIl19LCJzaWduX2luX3Byb3Zp"
		"ZGVyIjoiZ29vZ2xlLmNvbSJ9fQ.EySBWJhvCHzE4uTi8x3K3iMAFV08QZHMZO2mUesreHKePb_"
		"177BaNWAKKatsRXfQtHfPGEF6uW8l4qjqtSzWwg_a3k5gSr_F_z4okdYcS7EImDUp2S-"
		"b0RaNZhWmp5hzO4yD5qU7fjo4XU2iT5SHBU4yNqEYXQT-QpiIvEz15j2l-M1FmT_S7_"
		"whkRs2whMAXfBWjX634s0XxeeocPFQvoYip7DI8i9eEsakMD49UNRxHmTiVd56juSKlTj_"
		"rd5ioOK2fl_tZTe0OABw5MOV_-ysGdkvgxNBn48GFYKwo7ows-bIN217--2xKYj-A7F_"
		"oFtDZGXQMN3GH4JLdQeHnA"
	);

	http_request->SetHeader("Content-Type", "application/json");
	http_request->SetContentAsString(req_json_stream);

	http_request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid() &&
				 Response->GetContentType() == "application/json") {
				auto res_data = TArray<FSystemImplsReplaceResponse>{};
				auto res_str = Response->GetContentAsString();
				UE_LOG(LogTemp, Warning, TEXT("RESPONSE: %s"), *res_str);
				auto success = FJsonObjectConverter::JsonArrayStringToUStruct(
					res_str,
					&res_data,
					0,
					0
				);
				if(!success) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to parse response: %s"),
						*res_str
					);
					return;
				}

				for(auto item : res_data) {
					if(item.status == "OK") {
						UE_LOG(LogTemp, Log, TEXT("Successfully uploaded wasm!"));
						UE_LOG(LogTemp, Log, TEXT("Uploaded System Names:"));
						for(auto name : item.system_names) {
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
			} else {
				UE_LOG(
					LogTemp,
					Error,
					TEXT("Failed to make HTTP post request: %s"),
					LexToString(Response->GetFailureReason())
				);
				UE_LOG(LogTemp, Error, TEXT("%s"), *Response->GetContentAsString());
			}
		}
	);

	http_request->ProcessRequest();
}

auto FEcsactNetWasmModule::OnSettingsModified() -> bool {
	const auto* settings = GetDefault<UEcsactNetWasmSettings>();

	UE_LOG(LogTemp, Warning, TEXT("TODO: watch for wasm file changes"));
	// UploadSystemImpls();

	return true;
}

IMPLEMENT_MODULE(FEcsactNetWasmModule, EcsactNet)
