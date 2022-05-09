cd ~
mkdir steam
cd steam
mkdir tmp
cd tmp
wget https://cdn.cloudflare.steamstatic.com/client/installer/steam.deb
ar x steam.deb
tar xf data.tar.xz
cd ..
mv tmp/usr/* .
rm -rf tmp
cat > steam << EOF
#!/bin/bash
export STEAMOS=1
export STEAM_RUNTIME=1
~/steam/bin/steam steam://open/minigameslist
EOF
chmod +x steam
sudo mv steam /usr/local/bin/

