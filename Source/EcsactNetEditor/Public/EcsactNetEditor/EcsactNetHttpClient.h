#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "EcsactNetHttpClient.generated.h"

USTRUCT()

struct FEcsactNetAuthJson {
	GENERATED_BODY()

	UPROPERTY()
	FString display_name;

	UPROPERTY()
	FString email;

	UPROPERTY()
	FString photo_url;

	UPROPERTY()
	FString id_token;

	UPROPERTY()
	FString refresh_token;
};

USTRUCT()

struct FEcsactLoginAuthPayload {
	GENERATED_BODY()

	UPROPERTY()
	FString displayName;

	UPROPERTY()
	FString email;

	UPROPERTY()
	FString photoURL;

	UPROPERTY()
	FString idToken;

	UPROPERTY()
	FString refreshToken;
};

USTRUCT()

struct FEcsactRefreshTokenResponse {
	GENERATED_BODY()

	UPROPERTY()
	FString expires_in;

	UPROPERTY()
	FString id_token;

	UPROPERTY()
	FString refresh_token;
};

USTRUCT()

struct FSystemImplsReplaceRequest {
	GENERATED_BODY()

	UPROPERTY()
	FString fileContentsType;

	UPROPERTY()
	FString fileContents;
};

USTRUCT()

struct FSystemImplsReplaceResponse {
	GENERATED_BODY()

	UPROPERTY()
	FString status;

	UPROPERTY()
	TArray<FString> systemNames;
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

	auto RefreshIdToken( //
		FString                                      RefreshToken,
		TDelegate<void(FEcsactRefreshTokenResponse)> OnDone
	) -> void;

	auto GetAuthJson() -> TOptional<FEcsactNetAuthJson>;

	auto GetAuthTokenAsync( //
		TDelegate<void(FString)> Callback
	) -> void;

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

	auto ReplaceEcsactFiles(TDelegate<void()> OnDone) -> void;
};
