#include "EcsactNetWasm.h"
#include "Modules/ModuleManager.h"
#include "EcsactNetWasmSettings.h"

#define LOCTEXT_NAMESPACE "FEcsactNetWasmModule"

auto FEcsactNetWasmModule::StartupModule() -> void {
}

auto FEcsactNetWasmModule::ShutdownModule() -> void {
}

IMPLEMENT_MODULE(FEcsactNetWasmModule, EcsactNetWasm)
