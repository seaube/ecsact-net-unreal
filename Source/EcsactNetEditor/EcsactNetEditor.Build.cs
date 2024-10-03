using UnrealBuildTool;
using EpicGames.Core;

public class EcsactNetEditor : ModuleRules {
	public EcsactNetEditor(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

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
			"EcsactEditor",
		});
	}
}
