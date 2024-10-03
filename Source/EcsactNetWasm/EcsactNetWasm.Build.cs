using UnrealBuildTool;
using System.IO;

public class EcsactNetWasm : ModuleRules {
	public EcsactNetWasm(ReadOnlyTargetRules Target) : base(Target) {
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
			"Http",
			"Json",
			"JsonUtilities",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"EcsactNetEditor",
			"EcsactEditor",
		});

		DynamicallyLoadedModuleNames.AddRange(new string[] {});
	}
}
