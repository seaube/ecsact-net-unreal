#pragma once

#include "CoreMinimal.h"
#include "EcsactNetHttpClient.generated.h"

USTRUCT()
struct FEcsactNetAuthJson {
	GENERATED_BODY()

	UPROPERTY()
	FString id_token;
};

UCLASS()
class UEcsactNetHttpClient : public UObject {
	GENERATED_BODY()
};
