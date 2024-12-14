#include "EcsactNetDeveloperConnectBlueprintAction.h"
#if WITH_EDITOR
#	include "EcsactNetEditor/EcsactNetSettings.h"
#	include "EcsactNetEditor/EcsactNetEditor.h"
#endif

auto UEcsactNetDeveloperConnectBlueprintAction::DeveloperConnect(
	const UObject* WorldContext
) -> UEcsactNetDeveloperConnectBlueprintAction* {
	auto action = NewObject<UEcsactNetDeveloperConnectBlueprintAction>();
	action->WorldContext = WorldContext;
	return action;
}

auto UEcsactNetDeveloperConnectBlueprintAction::Activate() -> void {
#if WITH_EDITOR
	const auto* settings = GetDefault<UEcsactNetSettings>();
	const auto  NodeId = settings->NodeId;

	auto req = FNodeAuthRequest{.nodeId = NodeId, .address = "0.0.0.0"};
	FEcsactNetEditorModule::Get().GetHttpClient()->NodeAuth(
		req,
		TDelegate<void(FNodeAuthResponse)>::CreateLambda( //
			[this](FNodeAuthResponse response) {
				ConnectRequest(TCHAR_TO_UTF8(*response.nodeConnectionUri));
			}
		)
	);

#else
	UE_LOG(
		LogTemp,
		Error,
		TEXT("EcsactNet DeveloperConnect is only available in editor builds")
	);
#endif
}

auto UEcsactNetDeveloperConnectBlueprintAction::GetWorld() const -> UWorld* {
	return WorldContext->GetWorld();
}