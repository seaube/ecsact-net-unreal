#include "EcsactNetHttpClient.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "JsonObjectConverter.h"
#include "Misc/Base64.h"
#include "EcsactNetEditor/EcsactNetSettings.h"

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
	if(!AuthToken.IsEmpty()) {
		req->SetHeader("ecsact-net-token", AuthToken);
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

	http_request->ProcessRequest();
}