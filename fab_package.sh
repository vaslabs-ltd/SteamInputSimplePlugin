cd Plugins
directories_to_keep="SteamInputSimplePlugin/Config/\
 SteamInputSimplePlugin/Content\
 SteamInputSimplePlugin/Resources\
 SteamInputSimplePlugin/Source\
 SteamInputSimplePlugin/SteamInputSimplePlugin.uplugin"

zip -r -9 -X -q -o "SteamInputPlugin.zip" $directories_to_keep

[ -d ../target ] && rm -r ../target

mkdir -p ../target/fab

mv "SteamInputPlugin.zip" ../target/fab/SteamInputPlugin.zip

cd -