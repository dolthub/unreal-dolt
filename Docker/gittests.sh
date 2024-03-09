rm -rf /home/ue4/UnrealEngine/DoltPlugin/.git
sudo cp -r /home/ue4/TestData/gitserver/.git /home/ue4/UnrealEngine/DoltPlugin/.git
git config --global --add safe.directory /home/ue4/UnrealEngine/DoltPlugin
