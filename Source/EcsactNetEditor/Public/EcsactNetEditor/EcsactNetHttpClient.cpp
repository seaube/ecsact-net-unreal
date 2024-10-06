#include "EcsactNetHttpClient.h"
#include "EcsactNetEditor/EcsactNetEditorUtil.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/Archive.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "JsonWebToken.h"
#include "Misc/Base64.h"
#include "EcsactNetEditor/EcsactNetSettings.h"
#include "EcsactNetEditorPayloads.h"
#include "EcsactEditor.h"
#include "EcsactNetEditor.h"

auto UEcsactNetHttpClient::CreateRequest( //
	FString Endpoint
) -> TSharedRef<IHttpRequest, ESPMode::ThreadSafe> {
	const auto* settings = GetDefault<UEcsactNetSettings>();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> req =
		FHttpModule::Get().CreateRequest();

	auto endpoint_prefix = settings->EndpointPrefix;
	if(endpoint_prefix.IsEmpty()) {
		endpoint_prefix = "https://api.ecsact-net.seaube.com";
	}

	// NOTE: It's common for the prefix to accidentally (or on purpose) have a
	// trailing slash. For convenience we support both a trailing slash and no
	// trailing slash.
	endpoint_prefix.RemoveFromEnd("/");
	if(!Endpoint.StartsWith("/")) {
		Endpoint = "/" + Endpoint;
	}

	req->SetHeader("Content-Type", "application/json");
	req->SetURL(endpoint_prefix + Endpoint);
	if(!settings->ProjectID.IsEmpty()) {
		req->SetHeader("project_id", settings->ProjectID);
	}

	return req;
}

auto UEcsactNetHttpClient::SetAuthToken(FString InAuthToken) -> void {
	AuthToken = InAuthToken;
}

template<typename T>
static auto UStructArrayToJsonArray(const TArray<T> Array) -> FString {
	auto req_json_stream = FString{"["};

	for(auto item : Array) {
		auto req_json = FString{};
		auto serialize_success =
			FJsonObjectConverter::UStructToJsonObjectString(item, req_json);

		if(!serialize_success) {
			UE_LOG(LogTemp, Error, TEXT("Failed to serialize request payload"));
			return {};
		}

		req_json_stream += "  " + req_json + ",\n";
	}
	req_json_stream.RemoveFromEnd(",\n");
	req_json_stream += "\n]";
	return req_json_stream;
}

auto UEcsactNetHttpClient::GetAuthTokenAsync( //
	TDelegate<void(FString)> Callback
) -> void {
	auto auth_json = GetAuthJson();
	if(!auth_json) {
		UE_LOG(LogTemp, Error, TEXT("No auth json available"));
		return;
	}

	auto web_token = FJsonWebToken::FromString(auth_json->id_token);
	if(!web_token) {
		UE_LOG(LogTemp, Error, TEXT("Invalid web token"));
		return;
	}

	if(web_token->HasExpired()) {
		RefreshIdToken(
			auth_json->refresh_token,
			TDelegate<void(FEcsactRefreshTokenResponse)>::CreateLambda(
				[Callback = std::move(Callback)]( //
					FEcsactRefreshTokenResponse Res
				) -> void { Callback.ExecuteIfBound(Res.id_token); }
			)
		);
	} else {
		Callback.ExecuteIfBound(auth_json->id_token);
	}
}

auto UEcsactNetHttpClient::GetAuthJson() -> TOptional<FEcsactNetAuthJson> {
	auto auth_json_path = EcsactNetEditorUtil::GetAuthJsonPath();
	auto auth_json_str = FString{};

	if(!FFileHelper::LoadFileToString(auth_json_str, *auth_json_path)) {
		return {};
	}

	auto auth_json = FEcsactNetAuthJson{};
	auto success = FJsonObjectConverter::JsonObjectStringToUStruct( //
		auth_json_str,
		&auth_json
	);

	if(!success) {
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize auth json"));
		return {};
	}

	if(auth_json.id_token.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("id_token is empty: %s"), *auth_json_str);
		return {};
	}
	if(auth_json.refresh_token.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("refresh_token is empty"));
		return {};
	}

	return auth_json;
}

auto UEcsactNetHttpClient::RefreshIdToken(
	FString                                      RefreshToken,
	TDelegate<void(FEcsactRefreshTokenResponse)> OnDone
) -> void {
	constexpr auto api_key = "AIzaSyBKeB1T-abSePIotAnvIKATvInXTfi8UVM";
	constexpr auto hostname = "securetoken.googleapis.com";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> req =
		FHttpModule::Get().CreateRequest();

	req->SetURL(FString::Format( //
		TEXT("https://{}/v1/token?key={}"),
		FStringFormatOrderedArguments{hostname, api_key}
	));
	req->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	req->SetContentAsString(FString::Format( //
		TEXT("grant_type=refresh_token&refresh_token={}"),
		FStringFormatOrderedArguments{RefreshToken}
	));

	req->OnProcessRequestComplete().BindLambda( //
		[OnDone = std::move(OnDone)](
			FHttpRequestPtr  Request,
			FHttpResponsePtr Response,
			bool             ConnectedSuccessfully
		) -> void {
			auto payload = FEcsactRefreshTokenResponse{};
			auto success = FJsonObjectConverter::JsonObjectStringToUStruct(
				Response->GetContentAsString(),
				&payload
			);
			if(!success) {
				UE_LOG(LogTemp, Error, TEXT("TODO: handle this error"));
				return;
			}
			OnDone.ExecuteIfBound(payload);
		}
	);

	req->ProcessRequest();
}

auto UEcsactNetHttpClient::UploadSystemImpls( //
	TArray<FSystemImplsReplaceRequest> Requests,
	FOnUploadSystemImplsDone           OnDone
) -> void {
	auto http_request = CreateRequest("/v1/project/system-impls/replace");
	http_request->SetVerb("POST");
	http_request->SetContentAsString(UStructArrayToJsonArray(Requests));
	http_request->OnProcessRequestComplete().BindLambda(
		[OnDone = std::move(OnDone
		 )](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid() &&
				 Response->GetContentType() == "application/json") {
				auto res_data = TArray<FSystemImplsReplaceResponse>{};
				auto res_str = Response->GetContentAsString();
				UE_LOG(LogTemp, Warning, TEXT("RESPONSE: %s"), *res_str);
				auto success = FJsonObjectConverter::JsonArrayStringToUStruct(
					res_str,
					&res_data,
					0,
					0
				);
				if(!success) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to parse response: %s"),
						*res_str
					);
					return;
				}

				OnDone.ExecuteIfBound(std::move(res_data));
			} else {
				if(Response) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to make HTTP post: %s"),
						LexToString(Response->GetFailureReason())
					);
					UE_LOG(LogTemp, Error, TEXT("%s"), *Response->GetContentAsString());
				} else {
					UE_LOG(LogTemp, Error, TEXT("Failed to make HTTP request"));
				}
			}
		}
	);

	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request = std::move(http_request)](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}

auto UEcsactNetHttpClient::ReplaceEcsactFiles( //
	TDelegate<void()> OnDone
) -> void {
	auto settings = GetDefault<UEcsactNetSettings>();
	auto http_request = CreateRequest("/v1/project/ecsact/replace");

	http_request->SetVerb("POST");
	UE_LOG(
		LogTemp,
		Log,
		TEXT("ReplaceEcasctRequest with Project id %s"),
		*settings->ProjectID
	);

	auto ecsact_file_paths = FEcsactEditorModule::GetAllEcsactFiles();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Sending %i ecsact files"),
		ecsact_file_paths.Num()
	);

	auto ecsact_replace_reqs = TArray<FEcsactReplaceRequest>{};

	for(auto ecsact_file_path : ecsact_file_paths) {
		auto ecsact_contents = FString{};

		FFileHelper::LoadFileToString(ecsact_contents, *ecsact_file_path);

		UE_LOG(LogTemp, Log, TEXT("JSON %s"), *ecsact_contents);
		ecsact_replace_reqs.Add({.file_str = ecsact_contents});
	}

	auto ecsact_files_json = UStructArrayToJsonArray(ecsact_replace_reqs);

	http_request->SetContentAsString(ecsact_files_json);

	http_request->OnProcessRequestComplete().BindLambda( //
		[OnDone = std::move(OnDone)](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid()) {
				auto res_str = Response->GetContentAsString();
				auto res_data = TArray<FEcsactReplaceResponse>{};

				auto success = FJsonObjectConverter::JsonArrayStringToUStruct(
					res_str,
					&res_data,
					0,
					false
				);

				if(!success) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to parse response: %s"),
						*res_str
					);
					return;
				}

				for(auto item : res_data) {
					if(item.result) {
						UE_LOG(LogTemp, Log, TEXT("Successfully replaced Ecsact file!"));
						UE_LOG(
							LogTemp,
							Log,
							TEXT("NODE BUILD ID: %s"),
							*item.node_build_id
						);
					}
				}

				OnDone.ExecuteIfBound();
			}
		}
	);

	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request = std::move(http_request)](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}
