FROM ghcr.io/epicgames/unreal-engine:dev-4.27

# Build the project
RUN /home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -utf8output -platform=Linux -clientconfig=Shipping -serverconfig=Shipping -project=/project/DoltPlugin.uproject -noP4 -nodebuginfo -allmaps -build -prereqs

# Include the git local and remote git directories for the git tests

# Open the project in the editor and monitor the test results
CMD /home/ue4/UnrealEngine/Engine/Binaries/Linux/UE4Editor /project/DoltPlugin.uproject -ExecCmds="RunDoltTests" -nullrhi -unattended -log -abslog=/project/Logs/Editor.log -stdout -ForceResavePackage -NoVerifyGC -NoVerify -NoSplash -NoLogTimes -NoLoadStartupPackages