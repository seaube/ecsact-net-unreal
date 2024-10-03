#pragma once

#include "EcsactNetWasmUpload.generated.h"

USTRUCT()

struct FSystemImplsReplaceRequest {
	GENERATED_BODY()

	UPROPERTY()
	FString file_contents_type;

	UPROPERTY()
	FString file_contents;
};

USTRUCT()

struct FSystemImplsReplaceResponse {
	GENERATED_BODY()

	UPROPERTY()
	FString status;

	UPROPERTY()
	TArray<FString> system_names;
};
