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

/**
 * Prepare unary grpc http request. (1 request, 1 response)
 */
template<typename RequestPayloadT, typename ResponsePayloadT>
static auto PrepareUnary(
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest,
	RequestPayloadT                               RequestPayload,
	TDelegate<void(ResponsePayloadT)>             OnDone
) -> void {
	auto req_str = FString{};
	auto serialize_success =
		FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, req_str);
	if(!serialize_success) {
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize request"));
		return;
	}
	HttpRequest->SetContentAsString(req_str);
	HttpRequest->OnProcessRequestComplete().BindLambda( //
		[OnDone = std::move(OnDone
		 )](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid()) {
				auto res = ResponsePayloadT{};
				auto deserialize_success =
					FJsonObjectConverter::JsonObjectStringToUStruct(
						Response->GetContentAsString(),
						&res
					);
				if(!deserialize_success) {
					UE_LOG(LogTemp, Error, TEXT("Failed to deserialize response"));
					return;
				}
				OnDone.ExecuteIfBound(res);
			} else {
				UE_LOG(LogTemp, Error, TEXT("Node auth request failed"));
			}
		}
	);
}

/**
 * Prepare bidi grpc http request. (many requests, many responses)
 */
template<typename RequestPayloadT, typename ResponsePayloadT>
static auto PrepareBidi(
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest,
	TArray<RequestPayloadT>                       RequestPayloads,
	TDelegate<void(TArray<ResponsePayloadT>)>     OnDone
) -> void {
	HttpRequest->SetContentAsString(UStructArrayToJsonArray(RequestPayloads));
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[OnDone = std::move(OnDone
		 )](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid() &&
				 Response->GetContentType() == "application/json") {
				auto res_data = TArray<ResponsePayloadT>{};
				auto res_str = Response->GetContentAsString();
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
}

/**
 * Prepare client streaming grpc http request. (many requests, one response)
 */
template<typename RequestPayloadT, typename ResponsePayloadT>
static auto PrepareClientStreaming(
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest,
	TArray<RequestPayloadT>                       RequestPayloads,
	TDelegate<void(ResponsePayloadT)>             OnDone
) -> void {
	HttpRequest->SetContentAsString(UStructArrayToJsonArray(RequestPayloads));
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[OnDone = std::move(OnDone
		 )](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid()) {
				auto res = ResponsePayloadT{};
				auto deserialize_success =
					FJsonObjectConverter::JsonObjectStringToUStruct(
						Response->GetContentAsString(),
						&res
					);
				if(!deserialize_success) {
					UE_LOG(LogTemp, Error, TEXT("Failed to deserialize response"));
					return;
				}
				OnDone.ExecuteIfBound(res);
			} else {
				UE_LOG(LogTemp, Error, TEXT("Client streaming response fail"));
			}
		}
	);
}

/**
 * Prepare server streaming grpc http request. (one request, many responses)
 */
template<typename RequestPayloadT, typename ResponsePayloadT>
static auto PrepareServerStreaming(
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest,
	RequestPayloadT                               RequestPayload,
	TDelegate<void(TArray<ResponsePayloadT>)>     OnDone
) -> void {
	auto req_str = FString{};
	auto serialize_success =
		FJsonObjectConverter::UStructToJsonObjectString(RequestPayload, req_str);
	if(!serialize_success) {
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize request"));
		return;
	}
	HttpRequest->SetContentAsString(req_str);
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[OnDone = std::move(OnDone
		 )](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) {
			if(bSuccess && Response.IsValid() &&
				 Response->GetContentType() == "application/json") {
				auto res_data = TArray<ResponsePayloadT>{};
				auto res_str = Response->GetContentAsString();
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

auto UEcsactNetHttpClient::SaveAuthJson(FEcsactNetAuthJson AuthJson) -> void {
	auto auth_json_path = EcsactNetEditorUtil::GetAuthJsonPath();

	auto auth_json_str = FString{};
	auto serialize_success =
		FJsonObjectConverter::UStructToJsonObjectString(AuthJson, auth_json_str);
	check(serialize_success);

	auto write_auth_json_success =
		FFileHelper::SaveStringToFile(auth_json_str, *auth_json_path);

	UE_LOG(EcsactNetEditor, Log, TEXT("Updated %s"), *auth_json_path);
	UE_LOG(
		EcsactNetEditor,
		Log,
		TEXT("Successfully logged in as %s (%s)"),
		*AuthJson.display_name,
		*AuthJson.email
	);
}

auto UEcsactNetHttpClient::RefreshIdToken(
	FString                                      RefreshToken,
	TDelegate<void(FEcsactRefreshTokenResponse)> OnDone
) -> void {
	constexpr auto api_key = "AIzaSyBKeB1T-abSePIotAnvIKATvInXTfi8UVM";

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> req =
		FHttpModule::Get().CreateRequest();

	req->SetVerb("POST");
	req->SetURL(FString::Format( //
		TEXT("https://securetoken.googleapis.com/v1/token?key={0}"),
		FStringFormatOrderedArguments{api_key}
	));
	req->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	req->SetHeader("Hostname", "localhost:8080");
	req->SetHeader("Referer", "http://localhost:8080");
	req->SetContentAsString(FString::Format( //
		TEXT("grant_type=refresh_token&refresh_token={0}"),
		FStringFormatOrderedArguments{RefreshToken}
	));

	req->OnProcessRequestComplete().BindLambda( //
		[this, OnDone = std::move(OnDone)](
			FHttpRequestPtr  Request,
			FHttpResponsePtr Response,
			bool             ConnectedSuccessfully
		) -> void {
			if(ConnectedSuccessfully && Response.IsValid()) {
				auto payload = FEcsactRefreshTokenResponse{};
				auto payload_str = Response->GetContentAsString();
				auto success = FJsonObjectConverter::JsonObjectStringToUStruct(
					payload_str,
					&payload
				);
				if(!success) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to parse refresh token response")
					);
					return;
				}

				if(payload.error.code) {
					UE_LOG(
						LogTemp,
						Error,
						TEXT("Failed to refresh token auth: %s"),
						*payload.error.message
					);
					return;
				}

				auto current_auth_json = GetAuthJson().Get({});
				current_auth_json.id_token = payload.id_token;
				current_auth_json.refresh_token = payload.refresh_token;
				SaveAuthJson(current_auth_json);
				OnDone.ExecuteIfBound(payload);
			} else {
				UE_LOG(
					LogTemp,
					Error,
					TEXT("TODO: handle this error. may need to login again?")
				);
			}
		}
	);

	req->ProcessRequest();
}

auto UEcsactNetHttpClient::UploadSystemImpls( //
	TArray<FSystemImplsReplaceRequest>                   Requests,
	TDelegate<void(TArray<FSystemImplsReplaceResponse>)> OnDone
) -> void {
	auto http_request = CreateRequest("/v1/project/system-impls/replace");
	http_request->SetVerb("POST");
	PrepareBidi(http_request, Requests, OnDone);
	http_request->SetContentAsString(UStructArrayToJsonArray(Requests));
	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request = std::move(http_request)](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}

auto UEcsactNetHttpClient::ReplaceEcsactFiles( //
	TArray<FEcsactReplaceRequest>           Requests,
	TDelegate<void(FEcsactReplaceResponse)> OnDone
) -> void {
	auto http_request = CreateRequest("/v1/project/ecsact/replace");
	http_request->SetVerb("POST");
	PrepareClientStreaming(http_request, Requests, OnDone);
	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request = std::move(http_request)](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}

auto UEcsactNetHttpClient::NodeAuth(
	FNodeAuthRequest                   Request,
	TDelegate<void(FNodeAuthResponse)> OnDone
) -> void {
	auto http_request = CreateRequest("/v1/node/auth");
	http_request->SetVerb("POST");
	PrepareUnary(http_request, Request, OnDone);
	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}

auto UEcsactNetHttpClient::NodeList(
	FNodeListRequest                   Request,
	TDelegate<void(TArray<FNodeInfo>)> OnDone
) -> void {
	auto http_request = CreateRequest("/v1/node/list");
	http_request->SetVerb("POST");
	PrepareServerStreaming(http_request, Request, OnDone);
	GetAuthTokenAsync(TDelegate<void(FString)>::CreateLambda(
		[this, http_request](FString AuthToken) -> void {
			http_request->SetHeader("ecsact-net-token", AuthToken);
			http_request->ProcessRequest();
		}
	));
}
