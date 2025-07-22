// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteamInputSimplePlugin.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#define LOCTEXT_NAMESPACE "FSteamInputSimplePluginModule"

void FSteamInputSimplePluginModule::StartupModule()
{}

void FSteamInputSimplePluginModule::ShutdownModule()
{}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSteamInputSimplePluginModule, SteamInputSimplePlugin)
