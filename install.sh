# This will install Charl's dependencies and compile it, assuming you have a
# compiler. Read before running!

# If you get an issue running it, asking for a shared enet library, remove   
# that shared enet library file from /usr/local/lib and recompile.

sudo apt-get install libenet-dev libgtk2.0-dev
wget https://download.libsodium.org/libsodium/releases/libsodium-0.4.2.tar.gz
tar -xzvf libsodium-0.4.tar.gz
cd libsodium-0.4
./configure
make
sudo make install
cd ..
git clone https://github.com/PaytonTurnage/Charl.git
cd Charl
sh unix.sh
sudo mv client /usr/bin/charl
