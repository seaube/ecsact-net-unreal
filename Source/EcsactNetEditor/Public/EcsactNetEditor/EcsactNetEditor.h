#pragma once

#include "CoreMinimal.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Modules/ModuleManager.h"
#include "HttpResultCallback.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNetEditor, Log, All);

class ECSACTNETEDITOR_API FEcsactNetEditorModule : public IModuleInterface {
	class UEcsactNetHttpClient*    HttpClient;
	TSharedPtr<class FExtender>    MenuExtender;
	TArray<FMenuExtensionDelegate> ToolMenuExtensionDelegates;

	auto OnSettingsModified() -> bool;
	auto AddMenuEntry(class FMenuBuilder&) -> void;
	auto ReceivedAuthCallback(
		const struct FHttpServerRequest& Request,
		const FHttpResultCallback&       OnComplete
	) -> bool;

public:
	FEcsactNetEditorModule();
	~FEcsactNetEditorModule();

	auto AddEcsactNetToolsMenuExtension(
		const FMenuExtensionDelegate& MenuExtensionDelegate
	) -> void;

	auto StartupModule() -> void override;
	auto ShutdownModule() -> void override;

	/**
	 * Singleton-like access to the ecsact net editor module
	 */
	static auto Get() -> FEcsactNetEditorModule&;

	/**
	 * Http client for communicating with the ecsact net server.
	 *
	 * NOTE: only to be used internally.
	 */
	auto GetHttpClient() -> class UEcsactNetHttpClient*;
	auto GetHttpClient() const -> const class UEcsactNetHttpClient*;

	/**
	 * Start authentication process after which @ref GetAuthToken returns a valid
	 * authenticated token.
	 */
	auto Login() -> void;

	/**
	 * Get the current auth token. If not authenticated an empty string is
	 * returned. Does not guarantee the token is not expired.
	 */
	auto GetAuthToken() -> FString;
};
