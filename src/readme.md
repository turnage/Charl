Charl
================================================================================
A simple and secure way to chat.

Install
--------------------------------------------------------------------------------
Charl can be built on many Linux distros, Windows, and OSX. It has three
dependencies to resolve first though. These are enet, sodium, and gtk2.

To set up on *buntu distros

    sudo apt-get install libenet-dev libgtk2.0-dev
    wget http://download.dnscrypt.org/libsodium/releases/libsodium-0.4.tar.gz
    cd libsodium-0.4
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

Usage
--------------------------------------------------------------------------------
The [Charl wiki](https://github.com/PaytonTurnage/Charl3/wiki) on github has manuals for both the client side and server side.

Warning
--------------------------------------------------------------------------------
To avoid any confusion about what Charl is: this is a high school independent
study project. I am not a crypto expert by any means; this is a project I'm
using to learn how to cryptosystems. I plan to continue building on it and
making it better for a long while into the future, but as of now, the only thing
I can gauruntee for it as far as security is: better than Yahoo! Messenger. That
said, the crypto is provided by sodium and NaCl, the authors of which **are** crypto
experts.

Contribution
--------------------------------------------------------------------------------
If you happen to use Charl, please take a moment to tell me how you felt about
it, good or bad. As the developer it's hard for me to evaluate it from a
usability point of view, so feedback is welcome! Just email me at 
paytonturnage@gmail.com.

Similarly, if you've got a patch you want to contribute, email me the git diff
or submit a pullrequest to github. You'll be credited and I'll give you a
complement!