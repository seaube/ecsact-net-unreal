#pragma once

#include "EcsactNetEditorPayloads.generated.h"

USTRUCT()

struct FEcsactReplaceRequest {
	GENERATED_BODY();

	UPROPERTY()
	FString file_str;
};

USTRUCT()

struct FEcsactReplaceResponse {
	GENERATED_BODY();

	UPROPERTY()
	bool result;

	UPROPERTY()
	FString node_build_id;
};
