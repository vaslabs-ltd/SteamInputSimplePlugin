// Copyright Vaslabs LTD 2025 All Rights Reserved.

#include "SteamInputSimplePlugin.h"
#include "SteamInputSimpleInternal.h"

DEFINE_LOG_CATEGORY(LogSteamInputSimplePlugin);

void FSteamInputSimplePluginModule::StartupModule()
{}

void FSteamInputSimplePluginModule::ShutdownModule()
{}

IMPLEMENT_MODULE(FSteamInputSimplePluginModule, SteamInputSimplePlugin)
