#include "EcsactNetUnreal/Developer/DeveloperConnect.h"
#include "Misc/CoreMisc.h"
#if WITH_EDITOR
	#include "EcsactNetEditor/EcsactNetSettings.h"
	#include "EcsactNetEditor/EcsactNetEditor.h"
#endif

auto EcsactNetUnreal::GetDeveloperConnectionString(
	TDelegate<void(std::string connection_string)> OnSuccess,
	TDelegate<void(std::string error_message)> OnFailure
) -> void {
#if WITH_EDITOR
	if(!IsRunningGame()) {
		const auto* settings = GetDefault<UEcsactNetSettings>();

		auto do_request = [OnSuccess = std::move(OnSuccess), OnFailure = std::move(OnFailure)](FString node_id) {
			if(node_id.IsEmpty()) {
				OnFailure.ExecuteIfBound("DeveloperConnect: No nodes available");
				return;
			}

			auto req = FNodeAuthRequest{.nodeId = node_id, .address = "0.0.0.0"};
			FEcsactNetEditorModule::Get().GetHttpClient()->NodeAuth(
				req,
				TDelegate<void(FNodeAuthResponse)>::CreateLambda( //
					[OnSuccess = std::move(OnSuccess), OnFailure = std::move(OnFailure)](FNodeAuthResponse response) mutable {
						if(response.nodeConnectionUri.IsEmpty()) {
							OnFailure.ExecuteIfBound("DeveloperConnect: No connection URI - do you have any ecsact net nodes running?");
						} else {
							OnSuccess.ExecuteIfBound(TCHAR_TO_UTF8(*response.nodeConnectionUri));
						}
					}
				)
			);
		};

		if(settings->NodeId.IsEmpty()) {
			FEcsactNetEditorModule::Get().GetHttpClient()->NodeList(
				FNodeListRequest{},
				TDelegate<void(TArray<FNodeInfo>)>::CreateLambda([do_request = std::move(do_request)](TArray<FNodeInfo> node_list) {
					for(const auto& info : node_list) {
						if(info.status == "running") {
							do_request(info.nodeId);
							return;
						}
					}

					do_request("");
				})
			);
			OnFailure.ExecuteIfBound("DeveloperConnect: No Node ID set in unreal Ecsact Net settings");
			return;
		} else {
			do_request(settings->NodeId);
		}

	} else {
		// TODO: if we move the http client to a 'developer' module we should be
		// able to connect here as well
		OnFailure.ExecuteIfBound("DeveloperConnect: only available while playing in editor");
	}
#else
	OnFailure.ExecuteIfBound("DeveloperConnect: only available in editor builds");
#endif
}
