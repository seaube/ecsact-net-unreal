#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "HttpResultCallback.h"

DECLARE_LOG_CATEGORY_EXTERN(EcsactNetEditor, Log, All);

class FEcsactNetEditorModule : public IModuleInterface {
	class UEcsactNetHttpClient* HttpClient;

	auto OnSettingsModified() -> bool;
	auto AddMenuEntry(class FMenuBuilder&) -> void;
	auto ReceivedAuthCallback(
		const struct FHttpServerRequest& Request,
		const FHttpResultCallback&       OnComplete
	) -> bool;

public:
	FEcsactNetEditorModule();
	~FEcsactNetEditorModule();

	auto StartupModule() -> void override;
	auto ShutdownModule() -> void override;

	/**
	 * Singleton-like access to the ecsact net editor module
	 */
	static auto Get() -> FEcsactNetEditorModule&;

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
