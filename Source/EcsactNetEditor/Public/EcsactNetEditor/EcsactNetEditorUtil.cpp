#include "EcsactNetEditorUtil.h"

#include "Serialization/JsonSerializer.h"

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
