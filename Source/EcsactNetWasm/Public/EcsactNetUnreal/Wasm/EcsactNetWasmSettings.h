#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EcsactNetWasmSettings.generated.h"

UCLASS(Config = EcsactNetWasm, DefaultConfig)

class ECSACTNETWASM_API UEcsactNetWasmSettings : public UObject {
	GENERATED_BODY() // NOLINT

public:
	UPROPERTY(EditAnywhere, Config, Category = "Ecsact Net Wasm")
	TArray<FFilePath> SystemImplWasmFiles;
};
