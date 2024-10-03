using UnrealBuildTool;
using EpicGames.Core;

public class EcsactNet : ModuleRules {
	public EcsactNet(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"Ecsact",
			"EcsactEditor",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"CoreUObject",
			"Engine",
		});
	}
}
