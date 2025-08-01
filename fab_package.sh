directories_to_keep="Config Content/Examples Plugins/SteamInputSimplePlugin/Config/\
 Plugins/SteamInputSimplePlugin/Content\
 Plugins/SteamInputSimplePlugin/Resources\
 Plugins/SteamInputSimplePlugin/Source\
 Plugins/SteamInputSimplePlugin/SteamInputSimplePlugin.uplugin\
 Source\
 SteamInputPlugin.uproject"

zip -r -9 -X -q -o "SteamInputPlugin.zip" $directories_to_keep

[ -d target ] && rm -r target

mkdir -p target/fab

mv "SteamInputPlugin.zip" target/fab/SteamInputPlugin.zip