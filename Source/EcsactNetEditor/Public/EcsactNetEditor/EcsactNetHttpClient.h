#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "EcsactNetHttpClient.generated.h"

USTRUCT()

struct FEcsactNetAuthJson {
	GENERATED_BODY()

	UPROPERTY()
	FString id_token;
};

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

UCLASS()

class ECSACTNETEDITOR_API UEcsactNetHttpClient : public UObject {
	GENERATED_BODY()

	FString AuthToken;

	/**
	 * Creates a request suitable for the ecsact net server. Project ID and token
	 * are already set.
	 */
	[[nodiscard]] auto CreateRequest( //
		FString Endpoint
	) -> TSharedRef<IHttpRequest, ESPMode::ThreadSafe>;

public:
	/**
	 * Set the auth token that should be used for each request.
	 */
	auto SetAuthToken(FString InAuthToken) -> void;

	DECLARE_DELEGATE_OneParam( // NOLINT
		FOnUploadSystemImplsDone,
		TArray<FSystemImplsReplaceResponse>
	);

	auto UploadSystemImpls( //
		TArray<FSystemImplsReplaceRequest> Requests,
		FOnUploadSystemImplsDone           OnDone
	) -> void;
};
