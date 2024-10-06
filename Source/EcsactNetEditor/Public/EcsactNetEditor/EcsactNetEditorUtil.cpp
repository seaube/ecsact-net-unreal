#include "EcsactNetEditorUtil.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#include "Serialization/JsonSerializer.h"

static auto home_dir() -> FString {
#ifdef _WIN32
	return std::getenv("USERPROFILE");
#else
	return std::getenv("HOME");
#endif
}

auto EcsactNetEditorUtil::TArrayToJson(TArray<FString> strings) -> FString {
	TArray<TSharedPtr<FJsonValue>> JsonValArray;

	for(const auto& str : strings) {
		JsonValArray.Add((MakeShared<FJsonValueString>(str)));
	}

	auto ecsact_files_json = FString{};

	TSharedRef<TJsonWriter<>> JsonWriter =
		TJsonWriterFactory<>::Create(&ecsact_files_json);

	FJsonSerializer::Serialize(JsonValArray, JsonWriter);
	return FString{};
}

auto EcsactNetEditorUtil::GetAuthJsonPath() -> FString {
	auto auth_json_path = FPaths::Combine( //
		home_dir(),
		".config",
		"ecsact-net",
		"auth.json"
	);
	return auth_json_path;
}
