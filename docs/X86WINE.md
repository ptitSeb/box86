# Installing Wine (and winetricks)
_TwisterOS users: Wine, winetricks, and Box86 are already installed in TwisterOS. You do not have to install anything._

_Raspberry Pi users: Wine requires a 3G/1G split memory kernel. Raspberry Pi OS for the Pi 4 already has a 3G/1G split kernel and works with Wine, but **the Pi 3B+ and earlier models have a 2G/2G kernel and require a custom-compiled 3G/1G kernel to get Wine working.**_

See installation steps below (in the [Examples](#examples) section).

Using Wine with Box86 allows (x86) Windows programs to run on ARM Linux computers (for x64, use [box64](https://github.com/ptitSeb/box64) & wine-amd64 with an `aarch64` processor).

Box86 needs `wine-i386` to be installed **manually** on ARM devices.  Even though `wine-armhf` is available in many repo's on ARM devices (ie using _apt-get_ will attempt to install `wine-armhf` by default), `wine-armhf` will not work with Box86.  Note that manual installation is required since using `multiarch` will result in your ARM device thinking it needs to install lots of i386 dependencies to make `wine-i386` work. The "twist" in Box86 is that Box86 "wraps" many of Wine's core Linux i386 libraries (.so files) so that their calls are interpretable by your ARM device's other armhf Linux system libraries.  Also note that wrapping libraries is an ongoing process throughout Box86 development and that some programs may not run properly until all of their i386 library dependencies are wrapped.

Installation files for Wine can be found in the [WineHQ repository](https://dl.winehq.org/wine-builds/debian/dists/buster/main/binary-i386/), the [TwisterOS FAQ](https://twisteros.com/faq.html) page, or the [PlayOnLinux website repository](https://www.playonlinux.com/wine/). Box86 requires the "i386" (x86) version of Wine even though we are installing it on an ARM processor.

## Overview
The general procedure for installing Wine for Box86 is to...
 - Download all the install files (.deb, .zip, or even .pol files) for the version of Wine you wish to install
 - Unzip or dpkg the install files into one folder
 - Move that folder to the directory that you wish Wine to run from (in TwisterOS, this is `~/wine/` by default)
 - If you are running box86/i386-wine on a 64bit ARM OS (aarch64), you will also have to install some extra armhf libraries.
 - Go to `/usr/local/bin` and make symlinks or scripts that will point to your main wine binaries (`wine`, `winecfg`, and `wineserver`).
 - Boot wine to create a new wineprefix (`wine wineboot`).
 - Download winetricks (which is simply a very complicated bash script), make it executable, then copy it to `/usr/local/bin`.

## Examples
### Installing Wine for Box86 on Raspberry Pi OS from Twister OS FAQ .tgz files
_Links from the [TwisterOS FAQ](https://twisteros.com/faq.html)_

This installs Wine v5.13 (which is the version of Wine that comes pre-installed in TwisterOS).  When submitting bug reports, other Box86 users may assume you are using this version of Wine unless you specify otherwise.
```
# Backup any old wine installations
sudo mv ~/wine ~/wine-old
sudo mv ~/.wine ~/.wine-old
sudo mv /usr/local/bin/wine /usr/local/bin/wine-old
sudo mv /usr/local/bin/wineboot /usr/local/bin/wineboot-old
sudo mv /usr/local/bin/winecfg /usr/local/bin/winecfg-old
sudo mv /usr/local/bin/wineserver /usr/local/bin/wineserver-old

# Download, extract wine, and install wine (last I checked, the Twister OS FAQ page had Wine 5.13-devel)
wget https://twisteros.com/wine.tgz -O ~/wine.tgz
tar -xzvf ~/wine.tgz
rm ~/wine.tgz # clean up

# Install shortcuts (make launcher & symlinks. Credits: grayduck, Botspot)
echo -e '#!/bin/bash\nsetarch linux32 -L '"$HOME/wine/bin/wine "'"$@"' | sudo tee -a /usr/local/bin/wine >/dev/null # Create a script to launch wine programs as 32bit only
#sudo ln -s ~/wine/bin/wine /usr/local/bin/wine # You could also just make a symlink, but box86 only works for 32bit apps at the moment
sudo ln -s ~/wine/bin/wineboot /usr/local/bin/wineboot
sudo ln -s ~/wine/bin/winecfg /usr/local/bin/winecfg
sudo ln -s ~/wine/bin/wineserver /usr/local/bin/wineserver
sudo chmod +x /usr/local/bin/wine /usr/local/bin/wineboot /usr/local/bin/winecfg /usr/local/bin/wineserver

# Boot wine (make fresh wineprefix in ~/.wine )
wine wineboot
```

### Installing Wine for Box86 on Raspberry Pi OS from WineHQ .deb files
_Links from the [WineHQ repo](https://dl.winehq.org/wine-builds/debian/dists/buster/main/binary-i386/)_

This install method allows you to install different versions of Wine.  You may install any version/branch of Wine that you wish.
```
### User-defined Wine version variables ################
# - Replace the variables below with your system's info.
# - Note that we need the i386 version for Box86 even though we're installing it on our ARM processor.
# - Wine download links from WineHQ: https://dl.winehq.org/wine-builds/

wbranch="devel" #example: devel, staging, or stable (wine-staging 4.5+ requires libfaudio0:i386 - see below)
wversion="7.1" #example: 7.1
wid="debian" #example: debian, ubuntu
wdist="bullseye" #example (for debian): bullseye, buster, jessie, wheezy, etc
wtag="-1" #example: -1 (some wine .deb files have -1 tag on the end and some don't)

########################################################

# Clean up any old wine instances
wineserver -k # stop any old wine installations from running
rm -rf ~/.cache/wine # remove old wine-mono/wine-gecko install files
rm -rf ~/.local/share/applications/wine # remove old program shortcuts

# Backup any old wine installations
sudo mv ~/wine ~/wine-old
sudo mv ~/.wine ~/.wine-old
sudo mv /usr/local/bin/wine /usr/local/bin/wine-old
sudo mv /usr/local/bin/wineboot /usr/local/bin/wineboot-old
sudo mv /usr/local/bin/winecfg /usr/local/bin/winecfg-old
sudo mv /usr/local/bin/wineserver /usr/local/bin/wineserver-old

# Download, extract wine, and install wine
cd ~/Downloads
wget https://dl.winehq.org/wine-builds/${wid}/dists/${wdist}/main/binary-i386/wine-${wbranch}-i386_${wversion}~${wdist}${wtag}_i386.deb # download
wget https://dl.winehq.org/wine-builds/${wid}/dists/${wdist}/main/binary-i386/wine-${wbranch}_${wversion}~${wdist}${wtag}_i386.deb # (required for wine_i386 if no wine64 / CONFLICTS WITH wine64 support files)
dpkg-deb -x wine-${wbranch}-i386_${wversion}~${wdist}${wtag}_i386.deb wine-installer # extract
dpkg-deb -x wine-${wbranch}_${wversion}~${wdist}${wtag}_i386.deb wine-installer
mv wine-installer/opt/wine* ~/wine # install
rm wine*.deb # clean up
rm -rf wine-installer # clean up

# Install shortcuts (make 32bit launcher & symlinks. Credits: grayduck, Botspot)
echo -e '#!/bin/bash\nsetarch linux32 -L '"$HOME/wine/bin/wine "'"$@"' | sudo tee -a /usr/local/bin/wine >/dev/null # Create a script to launch wine programs as 32bit only
#sudo ln -s ~/wine/bin/wine /usr/local/bin/wine # You could aslo just make a symlink, but box86 only works for 32bit apps at the moment
sudo ln -s ~/wine/bin/wineboot /usr/local/bin/wineboot
sudo ln -s ~/wine/bin/winecfg /usr/local/bin/winecfg
sudo ln -s ~/wine/bin/wineserver /usr/local/bin/wineserver
sudo chmod +x /usr/local/bin/wine /usr/local/bin/wineboot /usr/local/bin/winecfg /usr/local/bin/wineserver

# These packages are needed for running wine on a 64-bit RPiOS via multiarch
karch=$(uname -m)
if [ "$karch" = "aarch64" ] || [ "$karch" = "aarch64-linux-gnu" ] || [ "$karch" = "arm64" ] || [ "$karch" = "aarch64_be" ]; then
    sudo dpkg --add-architecture armhf && sudo apt-get update # enable multi-arch
    sudo apt-get install -y libasound2:armhf libc6:armhf libglib2.0-0:armhf libgphoto2-6:armhf libgphoto2-port12:armhf \
        libgstreamer-plugins-base1.0-0:armhf libgstreamer1.0-0:armhf libldap-2.4-2:armhf libopenal1:armhf libpcap0.8:armhf \
        libpulse0:armhf libsane1:armhf libudev1:armhf libusb-1.0-0:armhf libvkd3d1:armhf libx11-6:armhf libxext6:armhf \
        libasound2-plugins:armhf ocl-icd-libopencl1:armhf libncurses6:armhf libncurses5:armhf libcap2-bin:armhf libcups2:armhf \
        libdbus-1-3:armhf libfontconfig1:armhf libfreetype6:armhf libglu1-mesa:armhf libglu1:armhf libgnutls30:armhf \
        libgssapi-krb5-2:armhf libkrb5-3:armhf libodbc1:armhf libosmesa6:armhf libsdl2-2.0-0:armhf libv4l-0:armhf \
        libxcomposite1:armhf libxcursor1:armhf libxfixes3:armhf libxi6:armhf libxinerama1:armhf libxrandr2:armhf \
        libxrender1:armhf libxxf86vm1:armhf libc6:armhf libcap2-bin:armhf # to run wine-i386 through box86:armhf on aarch64
        # This list found by downloading...
        #	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/wine-devel-i386_7.1~bullseye-1_i386.deb
        #	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/winehq-devel_7.1~bullseye-1_i386.deb
        #	wget https://dl.winehq.org/wine-builds/debian/dists/bullseye/main/binary-i386/wine-devel_7.1~bullseye-1_i386.deb
        # then `dpkg-deb -I package.deb`. Read output, add `:armhf` to packages in dep list, then try installing them on Pi aarch64.
fi

# These packages are needed for running wine-staging on RPiOS (Credits: chills340)
sudo apt install libstb0 -y
cd ~/Downloads
wget -r -l1 -np -nd -A "libfaudio0_*~bpo10+1_i386.deb" http://ftp.us.debian.org/debian/pool/main/f/faudio/ # Download libfaudio i386 no matter its version number
dpkg-deb -xv libfaudio0_*~bpo10+1_i386.deb libfaudio
sudo cp -TRv libfaudio/usr/ /usr/
rm libfaudio0_*~bpo10+1_i386.deb # clean up
rm -rf libfaudio # clean up

# Boot wine (make fresh wineprefix in ~/.wine )
wine wineboot
```

## Installing winetricks
Winetricks is a bash script which makes it easier to install & configure any desired Windows core system software packages which may be dependencies for certain Windows programs.  You can either install it with `apt`, or manually using the steps below.

```
sudo apt-get install cabextract -y                                                                   # winetricks needs this installed
sudo mv /usr/local/bin/winetricks /usr/local/bin/winetricks-old                                      # Backup old winetricks
cd ~/Downloads && wget https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks # Download
sudo chmod +x winetricks && sudo mv winetricks /usr/local/bin/                                       # Install
```

Whenever we run winetricks, we must tell Box86 to supress its info banner, otherwise winetricks will crash.  Box86's info banner can be suppressed using the `BOX86_NOBANNER=1` environment variable before invoking winetricks (example: `BOX86_NOBANNER=1 winetricks`).

If `apt` installed a desktop menu shortcut for winetricks (or if you make your own desktop shortcut for winetricks) then you may need to edit that shortcut to include Box86's BOX86_NOBANNER=1 env var.  With your favorite text editor, edit `/usr/share/applications/winetricks.desktop` and change `Exec=winetricks --gui` to `Exec=env BOX86_NOBANNER=1 winetricks --gui`.

## Example commands
Here is an example of how we should run a winetricks command with box86:  
`BOX86_NOBANNER=1 winetricks -q corefonts vcrun2010 dotnet20sp1`  
_This command will silently install three packages in series: Windows core fonts, VC++ 2010 Runtimes, and .NET 2.0 SP1.  `-q` is the "install silent/quiet" command._

Whenever we run winetricks, we must suppress Box86's banner by typing `BOX86_NOBANNER=1` to prevent winetricks from crashing.  Invoking Box86's logging features (with `BOX86_LOG=1` or similar) will also cause winetricks to crash.  (If Box86 logging is necessary though, we can patch winetricks to avoid these crashes - see the *Troubleshooting* section) # future work.

For a list of all the different Windows packages & libraries that winetricks can help you install, run `winetricks list-all`  

## Other notes

### Wineprefixes (and Wine initialization)
When you first run or boot Wine (`wine wineboot`), Wine will create a new user environment within which to install Windows software.  This user environment is called a "wineprefix" (or "wine bottle") and is located (by default) in `~/.wine` (note that Linux folders with a `.` in front of them are "hidden" folders).  Think of a wineprefix as Wine's virtual 'harddrive' where it installs software and saves settings.  Wineprefixes are portable and deletable.  For more Wine documentation, see [WineHQ](https://www.winehq.org/documentation).

If you at any point corrupt something inside your default wineprefix, you can start "fresh" by deleting your `~/.wine` directory (with the `rm -rf ~/.wine` command) and boot wine again to create a new default wineprefix.

### Transplanting wineprefixes (side-loading)
If software isn't installing in Wine with Box86, but is installing for you in Wine on a regular x86 Linux computer, you can copy a wineprefix from your x86 Linux computer to the device you're running Box86 on. This is most easily done by tarring the `~/.wine` folder on your x86 Linux computer (`tar -cvf winebottle.tar ~/.wine`), transferring the tar file to your device, then un-tarring the tar file (`tar -xf winebottle.tar`) on your device running Box86 & Wine. Tarring the wineprefix preserves any symlinks that are inside it.

### Swapping out different versions of Wine
Some versions of Wine work better with certain software. It is best to install a version of Wine that is known to work with the software you would like to run. There are three main development branches of Wine you can pick from, referred to as wine-stable, wine-devel, and wine-staging. _Note that the wine-staging branch requires extra installation steps on Raspberry Pi._

Your entire Wine installation can reside within a single folder on your Linux computer. TwisterOS assumes that your Wine installation is located inside the `~/wine/` directory. The actual directory where you put your `wine` folder doesn't matter as long as you have symlinks within the `/usr/local/bin/` directory which point to the `wine` folder so that Linux can find Wine when you type `wine` into the terminal).

You can change which version of Wine you are running simply by renaming your old `wine` and `.wine` folders to something else, then putting a new `wine` folder (containing your new version of Wine) in its place. Running `wine wineboot` again will migrate a wineprefix from an older version of Wine to be usable by the newer version of Wine you just installed.  You can check which version of Wine you're running with the `wine --version` command.
