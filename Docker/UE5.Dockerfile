FROM dolthub/dolt as dolt

FROM ghcr.io/epicgames/unreal-engine:dev-5.3

COPY --from=dolt /usr/local/bin/dolt /usr/local/bin/dolt

RUN mkdir /home/ue4/dolt

RUN mkdir /home/ue4/UnrealEngine/DoltPlugin
RUN chmod 777 /home/ue4/UnrealEngine/DoltPlugin

COPY --chmod=777 ./Content /home/ue4/UnrealEngine/DoltPlugin/Content
COPY --chmod=777 ./Plugins /home/ue4/UnrealEngine/DoltPlugin/Plugins
COPY --chmod=777 ./Source /home/ue4/UnrealEngine/DoltPlugin/Source
COPY --chmod=777 ./DoltPlugin.uproject /home/ue4/UnrealEngine/DoltPlugin/DoltPlugin.uproject
COPY --chmod=777 ./Docker/gittests.sh /home/ue4/UnrealEngine/DoltPlugin/gittests.sh
COPY --chmod=777 ./TestData/ /home/ue4/TestData/

RUN mkdir /home/ue4/UnrealEngine/DoltPlugin/Intermediate
RUN chmod 777 /home/ue4/UnrealEngine/DoltPlugin/Intermediate

COPY --chmod=777 ./TestData/GitSourceControlSettings.ini /home/ue4/UnrealEngine/DoltPlugin/Saved/Config/LinuxEditor/SourceControlSettings.ini
COPY --chmod=777 ./TestData/DefaultEditor.ini /home/ue4/UnrealEngine/DoltPlugin/Config/DefaultEditor.ini

# Build the project
RUN /home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -utf8output -platform=Linux -clientconfig=Shipping -serverconfig=Shipping -project=/DoltPlugin/DoltPlugin.uproject -noP4 -nodebuginfo -allmaps -compile -build -prereqs

# Open the project in the editor and monitor the test results
WORKDIR /home/ue4/UnrealEngine/DoltPlugin/
RUN ./gittests.sh
CMD /home/ue4/UnrealEngine/Engine/Binaries/Linux/UnrealEditor /home/ue4/UnrealEngine/DoltPlugin/DoltPlugin.uproject -ExecCmds="RunAllDoltTests, Quit" -nullrhi -unattended -stdout -ForceResavePackage -NoVerifyGC -NoVerify -NoSplash -NoLogTimes -NoLoadStartupPackages