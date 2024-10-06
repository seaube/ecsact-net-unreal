#pragma once

#include "Containers/UnrealString.h"

namespace EcsactNetEditorUtil {
auto TArrayToJson(TArray<FString> strings) -> FString;
auto GetAuthJsonPath() -> FString;
} // namespace EcsactNetEditorUtil
