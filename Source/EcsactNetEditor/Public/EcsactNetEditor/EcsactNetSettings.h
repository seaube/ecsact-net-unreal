#pragma once

#include "CoreMinimal.h"
#include "EcsactNetSettings.generated.h"

UCLASS(Config = EcsactNet, DefaultConfig)

class ECSACTNETEDITOR_API UEcsactNetSettings : public UObject {
	GENERATED_BODY() // NOLINT
public:
	UEcsactNetSettings();

	/**
	 * Ecsact Net endpoint prefix.
	 *
	 * By default this should be https://api.seaube.com/v1/
	 */
	UPROPERTY(EditAnywhere, Config, Category = "API")
	FString EndpointPrefix;

	/**
	 * Ecsact Project ID
	 */
	UPROPERTY(EditAnywhere, Config, Category = "Project")
	FString ProjectID;

	UPROPERTY(EditAnywhere, Config, Category = "Project")
	FString NodeId;
};
