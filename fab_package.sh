cd Plugins/SteamInputSimplePlugin
directories_to_keep="Config/\
 Content/\
 Resources/\
 Source/\
 SteamInputSimplePlugin.uplugin"

target_dir="../../target"

zip -r -9 -X -q -o "SteamInputPlugin.zip" $directories_to_keep

[ -d $target_dir ] && rm -r $target_dir

mkdir -p $target_dir/fab

mv "SteamInputPlugin.zip" $target_dir/fab/SteamInputPlugin.zip

cd -