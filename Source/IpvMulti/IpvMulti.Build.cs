// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IpvMulti : ModuleRules
{
	public IpvMulti(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"IpvMulti",
			"IpvMulti/Variant_Platforming",
			"IpvMulti/Variant_Platforming/Animation",
			"IpvMulti/Variant_Combat",
			"IpvMulti/Variant_Combat/AI",
			"IpvMulti/Variant_Combat/Animation",
			"IpvMulti/Variant_Combat/Gameplay",
			"IpvMulti/Variant_Combat/Interfaces",
			"IpvMulti/Variant_Combat/UI",
			"IpvMulti/Variant_SideScrolling",
			"IpvMulti/Variant_SideScrolling/AI",
			"IpvMulti/Variant_SideScrolling/Gameplay",
			"IpvMulti/Variant_SideScrolling/Interfaces",
			"IpvMulti/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
