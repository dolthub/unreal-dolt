FROM ghcr.io/epicgames/unreal-engine:dev-5.3

RUN apt-get update
RUN apt-get dist-upgrade -y

# Get some dependencies for adding apt repositories

RUN apt-get install -y wget gnupg

# Add perforce repo

RUN wget -qO - https://package.perforce.com/perforce.pubkey | apt-key add -
RUN echo 'deb http://package.perforce.com/apt/ubuntu focal release' > /etc/apt/sources.list.d/perforce.list
RUN apt-get update

# Actually install perforce

RUN apt-get install -y helix-p4d

# Go into our directory, start Perforce, and view the log outputs

CMD chown -R perforce:perforce /perforce-data && cd /dbs && p4dctl start master && tail -F /perforce-data/logs/log

COPY --chmod=777 . /home/ue4/UnrealEngine/DoltPlugin/

# Build the project
RUN /home/ue4/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -utf8output -platform=Linux -clientconfig=Shipping -serverconfig=Shipping -project=/DoltPlugin/DoltPlugin.uproject -noP4 -nodebuginfo -allmaps -build -prereqs

# Include the git local and remote git directories for the git tests

# Open the project in the editor and monitor the test results
CMD /home/ue4/UnrealEngine/Engine/Binaries/Linux/UnrealEditor /home/ue4/UnrealEngine/DoltPlugin/DoltPlugin.uproject -ExecCmds="RunAllDoltTests" -nullrhi -unattended -stdout -ForceResavePackage -NoVerifyGC -NoVerify -NoSplash -NoLogTimes -NoLoadStartupPackages