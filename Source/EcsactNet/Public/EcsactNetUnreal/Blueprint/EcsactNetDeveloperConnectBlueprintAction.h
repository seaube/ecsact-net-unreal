#pragma once

#include <string>
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EcsactUnreal/Blueprint/EcsactAsyncConnectBlueprintAction.h"
#include "EcsactNetDeveloperConnectBlueprintAction.generated.h"

/**
 *
 */
UCLASS(HideCategories = ("Ecsact Runtime"))

class ECSACTNET_API UEcsactNetDeveloperConnectBlueprintAction
	: public UEcsactAsyncConnectBlueprintAction {
	GENERATED_BODY() // NOLINT

	const UObject* WorldContext;

public:
	/**
	 * Called everytime an async request is updated. Error parameter is only
	 * valid during `OnError`.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( // NOLINT
		FAsyncConnectDoneCallback,
		EAsyncConnectError,
		Error
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Ecsact Net Developer",
		Meta = (BlueprintInternalUseOnly = true, WorldContext = "WorldContext")
	)
	static UEcsactNetDeveloperConnectBlueprintAction* DeveloperConnect(
		const UObject* WorldContext
	);

	auto Activate() -> void override;

	auto GetWorld() const -> UWorld* override;
};
