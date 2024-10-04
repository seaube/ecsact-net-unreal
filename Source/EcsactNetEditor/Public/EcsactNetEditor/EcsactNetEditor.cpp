#include "EcsactNetEditor.h"
#include "Dom/JsonValue.h"
#include "EcsactNetEditor/EcsactNetEditorUtil.h"
#include "EcsactUnreal/Ecsact.h"
#include "Framework/Commands/UIAction.h"
#include "HAL/PlatformFileManager.h"
#include "HttpServerRequest.h"
#include "UObject/NoExportTypes.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "ISettingsContainer.h"
#include "Modules/ModuleManager.h"
#include "HttpModule.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
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
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "EcsactNetHttpClient.h"
#include "EcsactNetSettings.h"
#include "EcsactEditor.h"
#include "EcsactNetEditorUtil.h"
#include "EcsactNetEditorPayloads.h"

#define LOCTEXT_NAMESPACE "FEcsactNetEditorModule"

static auto GetUnusedPort() -> int32 {
	auto* socket_ss = ISocketSubsystem::Get();
	check(socket_ss != nullptr);

	auto socket =
		socket_ss->CreateUniqueSocket(NAME_Stream, "Used to generated unused port");
	check(socket.IsValid());
	auto bind_addr = socket_ss->CreateInternetAddr();
	bind_addr->SetIp(0);
	bind_addr->SetPort(0);
	socket->Bind(bind_addr.Get());
	socket->Listen(0);
	auto port = socket->GetPortNo();
	socket->Close();
	return port;
}

FEcsactNetEditorModule::FEcsactNetEditorModule() {
	HttpClient = nullptr;
}

FEcsactNetEditorModule::~FEcsactNetEditorModule() {
}

auto FEcsactNetEditorModule::StartupModule() -> void {
	HttpClient = NewObject<UEcsactNetHttpClient>();

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

	auto& level_editor_module =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"Tools",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(
			this,
			&FEcsactNetEditorModule::AddMenuEntry
		)
	);

	level_editor_module.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

auto FEcsactNetEditorModule::AddEcsactNetToolsMenuExtension(
	const FMenuExtensionDelegate& MenuExtensionDelegate
) -> void {
	ToolMenuExtensionDelegates.Add(MenuExtensionDelegate);
}

auto FEcsactNetEditorModule::AddMenuEntry(FMenuBuilder& MenuBuilder) -> void {
	MenuBuilder.BeginSection(
		"EcsactNetTools",
		LOCTEXT("EcsactNetToolsSectionTitle", "Ecsact Net")
	);
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("Login", "Login"),
			LOCTEXT("Login", "Login"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this] { Login(); }))
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("EcsactNetUploadEcsact", "Upload Ecsact Files"),
			LOCTEXT(
				"EcsactNetUploadEcsact",
				"Manually re-upload Eccsact Files and run a node build"
			),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this] {
				HttpClient->ReplaceEcsactFiles();
			}))
		);

		// These delegates are from other modules that want to put menu items in the
		// ecsact net tools section
		for(auto tool_menu_delegate : ToolMenuExtensionDelegates) {
			tool_menu_delegate.ExecuteIfBound(MenuBuilder);
		}
	}
	MenuBuilder.EndSection();
}

auto FEcsactNetEditorModule::GetHttpClient() -> UEcsactNetHttpClient* {
	return HttpClient;
}

auto FEcsactNetEditorModule::GetHttpClient() const
	-> const UEcsactNetHttpClient* {
	return HttpClient;
}

auto FEcsactNetEditorModule::ShutdownModule() -> void {
}

auto FEcsactNetEditorModule::OnSettingsModified() -> bool {
	return true;
}

auto FEcsactNetEditorModule::Get() -> FEcsactNetEditorModule& {
	return FModuleManager::Get().GetModuleChecked<FEcsactNetEditorModule>(
		"EcsactNetEditor"
	);
}

auto FEcsactNetEditorModule::ReceivedAuthCallback(
	const FHttpServerRequest&  Request,
	const FHttpResultCallback& OnComplete
) -> bool {
	UE_LOG(LogTemp, Warning, TEXT("Received Auth Callback"));
	return false;
}

auto FEcsactNetEditorModule::Login() -> void {
	auto  unused_port = GetUnusedPort();
	auto& http_module = FHttpModule::Get();
	auto& http_server_module = FHttpServerModule::Get();
	auto  router = http_server_module.GetHttpRouter(unused_port);
	check(router);
	check(router.Get());

	router.Get()->RegisterRequestPreprocessor(FHttpRequestHandler::CreateLambda(
		[](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
			-> bool {
			auto res = MakeUnique<FHttpServerResponse>();
			res->Headers.Add("Access-Control-Allow-Origin", {"*"});
			res->Code = EHttpServerResponseCodes::Ok;
			if(Request.Verb == EHttpServerRequestVerbs::VERB_OPTIONS) {
				res->Headers.Add(
					"Access-Control-Allow-Headers",
					{"Content-Type, Accept, X-Requested-With"}
				);
				res->Headers.Add("Access-Control-Allow-Methods", {"POST"});
			} else if(Request.Verb == EHttpServerRequestVerbs::VERB_POST) {
			}
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Request: %s"),
				*Request.RelativePath.GetPath()
			);
			OnComplete(std::move(res));
			return true;
		}
	));

	// router.Get()->BindRoute( //
	// 	FHttpPath{"/*"},
	// 	EHttpServerRequestVerbs::VERB_POST,
	// 	FHttpRequestHandler::CreateRaw(
	// 		this,
	// 		&FEcsactNetEditorModule::ReceivedAuthCallback
	// 	)
	// );

	http_server_module.StartAllListeners();

	UE_LOG(LogTemp, Warning, TEXT("Using port: %i"), unused_port);
	auto err = FString{};
	auto url = FString::Format(
		TEXT("https://seaube.com/auth?p={0}&integration=unreal"),
		FStringFormatOrderedArguments{unused_port}
	);
	FPlatformProcess::LaunchURL(*url, TEXT(""), &err);
	if(!err.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to launch auth url: %s"), *err);
		return;
	}
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
