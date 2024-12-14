using UnrealBuildTool;
using EpicGames.Core;

public class EcsactNet : ModuleRules {
	public EcsactNet(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Core",
			"Ecsact",
		});

		if (Target.bBuildEditor) {
			PrivateDependencyModuleNames.Add("EcsactNetEditor");
		}

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Engine",
		});
	}
}
