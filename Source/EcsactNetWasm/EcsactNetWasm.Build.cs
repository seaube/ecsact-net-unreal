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
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
		});

		DynamicallyLoadedModuleNames.AddRange(new string[] {});
	}
}
