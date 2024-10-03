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
}

auto FEcsactNetEditorModule::AddMenuEntry(FMenuBuilder& MenuBuilder) -> void {
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
	auto& http_module = FHttpModule::Get();
	auto& http_server_module = FHttpServerModule::Get();
	auto  router = http_server_module.GetHttpRouter(0);

	auto handler = FHttpRequestHandler{};
	handler.BindRaw(this, &FEcsactNetEditorModule::ReceivedAuthCallback);
	router.Get()->BindRoute( //
		FHttpPath{"/"},
		EHttpServerRequestVerbs::VERB_POST,
		handler
	);

	http_server_module.StartAllListeners();

	auto unused_port = GetUnusedPort();
	UE_LOG(LogTemp, Warning, TEXT("Using port: %i"), unused_port);
	auto err = FString{};
	auto params = FString::Format(
		TEXT("?p={0}&integration=unreal"),
		FStringFormatOrderedArguments{unused_port}
	);
	FPlatformProcess::LaunchURL(TEXT("https://seaube.com/auth"), *params, &err);
	if(err.IsEmpty()) {
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