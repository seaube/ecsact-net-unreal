
using UnrealBuildTool;
using System.IO;

public class EcsactNetWasmEditor : ModuleRules {
	public EcsactNetWasmEditor(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {});

		PrivateIncludePaths.AddRange(new string[] {});

		PublicDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"Json",
			"Core",
			"Engine",
			"UnrealEd",
			"Ecsact",
			"Settings",
			"PropertyEditor",
			"HTTP",
			"Json",
			"JsonUtilities",
			"EcsactNetWasm",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"EcsactNetEditor",
			"EcsactEditor",
		});

		DynamicallyLoadedModuleNames.AddRange(new string[] {});
	}
}
