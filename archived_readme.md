Charl
================================================================================

A chat system.

Download
--------------------------------------------------------------------------------
Binaries of Charl are available at [sourceforge](http://sourceforge.net/projects/charl).

Install [From Source]
--------------------------------------------------------------------------------
Charl can be built on many Linux distros, Windows, and OSX. It has three
dependencies to resolve first though. These are enet, sodium, and gtk2.

To set up on *buntu distros

    sudo apt-get install libenet-dev libgtk2.0-dev
    wget http://download.libsodium.org/libsodium/releases/libsodium-0.4.2.tar.gz
    tar -xzvf libsodium-0.4.2.tar.gz
    cd libsodium-0.4.2
    ./configure
    make
    sudo make install
    cd ..
    git clone https://github.com/PaytonTurnage/Charl.git
    cd Charl
    sh unix.sh

On Windows, you'll need to install Cygwin or Msys with pkg-config, gtk2, and
manually build and install sodium and enet. When that's done, just run

    sh win.sh

in Msys or Cygwin or the like.

On OSX you will also have to manually get sodium and enet and build/install them
with the configure ritual, and for gtk2 I reccomend using MacPorts or Homebrew.
Use the unix script for OSX as well.

    sudo port install gtk2
    wget http://download.libsodium.org/libsodium/releases/libsodium-0.4.2.tar.gz
    tar -xzvf libsodium-0.4.2.tar.gz
    cd libsodium-0.4.2
    ./configure
    make
    sudo make install
    wget http://enet.bespin.org/download/enet-1.3.7.tar.gz
    tar -xzvf enet-1.3.7.tar.gz
    cd enet-1.3.7
    ./configure
    make
    sudo make install
    git clone https://github.com/PaytonTurnage/Charl.git
    cd Charl
    sh unix.sh

Usage
--------------------------------------------------------------------------------
The [Charl wiki](https://github.com/PaytonTurnage/Charl3/wiki) on github has manuals for both the client side and server side.

Warning
--------------------------------------------------------------------------------
This is a high school project. I am not a crypto expert; this is a project I'm
using to learn how to cryptosystems.
