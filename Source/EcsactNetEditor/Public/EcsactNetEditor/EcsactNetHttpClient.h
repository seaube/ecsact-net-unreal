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
	FString expiresIn;

	UPROPERTY()
	FString idToken;

	UPROPERTY()
	FString refreshToken;
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

USTRUCT()

struct FNodeAuthRequest {
	GENERATED_BODY()

	UPROPERTY()
	FString nodeId;

	UPROPERTY()
	FString address;
};

USTRUCT()

struct FNodeAuthResponse {
	GENERATED_BODY()

	UPROPERTY()
	FString nodeConnectionUri;
};

USTRUCT()

struct FNodeListRequest {
	GENERATED_BODY()
};

USTRUCT()

struct FNodeHost {
	GENERATED_BODY()

	UPROPERTY()
	FString hostName;

	UPROPERTY()
	FString port;
};

USTRUCT()

struct FNodeBuild {
	GENERATED_BODY()

	UPROPERTY()
	FString id;

	UPROPERTY()
	FString status;
};

USTRUCT()

struct FNodeInfo {
	GENERATED_BODY()

	UPROPERTY()
	FNodeHost nodeHost;

	UPROPERTY()
	FString status;

	UPROPERTY()
	FString nodeId;

	/**
	 * nodeBuildId or nodeBuild is set
	 */
	UPROPERTY()
	FString nodeBuildId;

	/**
	 * nodeBuildId or nodeBuild is set
	 */
	UPROPERTY()
	FNodeBuild nodeBuild;
};

UCLASS()

class ECSACTNETEDITOR_API UEcsactNetHttpClient : public UObject {
	GENERATED_BODY()

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
	auto SaveAuthJson(FEcsactNetAuthJson AuthJson) -> void;

	auto GetAuthTokenAsync( //
		TDelegate<void(FString)> Callback
	) -> void;

public:
	auto UploadSystemImpls( //
		TArray<FSystemImplsReplaceRequest>                   Requests,
		TDelegate<void(TArray<FSystemImplsReplaceResponse>)> OnDone
	) -> void;

	auto ReplaceEcsactFiles(
		TArray<FEcsactReplaceRequest>           Requests,
		TDelegate<void(FEcsactReplaceResponse)> OnDone
	) -> void;

	/**
	 * Get a node connection URI. Should only be used during development.
	 */
	auto NodeAuth(
		FNodeAuthRequest                   Request,
		TDelegate<void(FNodeAuthResponse)> OnDone
	) -> void;

	auto NodeList(
		FNodeListRequest                   Request,
		TDelegate<void(TArray<FNodeInfo>)> OnDone
	) -> void;
};
