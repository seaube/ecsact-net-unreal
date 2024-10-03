#pragma once

#include "CoreMinimal.h"
#include "EcsactNetSettings.generated.h"

UCLASS(Config = EcsactNet, DefaultConfig)

class UEcsactNetSettings : public UObject {
	GENERATED_BODY() // NOLINT
public:
	UEcsactNetSettings();

	UPROPERTY(EditAnywhere, Config, Category = "Project")
	FString project_id;
};
