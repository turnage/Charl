file_links="-Isrc"

enet_links="-lenet"
sodium_links="-lsodium"
sodium_lib_path="-Wl,-rpath=/usr/local/lib"
gtk_links="`pkg-config --cflags --libs gtk+-2.0`"

serv_sources="src/server/main.c src/charl.c src/server/client.c src/host.c src/crypto.c src/server/parser.c"
serv_links="$enet_links $sodium_links"

cli_sources="src/client/main.c src/charl.c src/client/gui.c src/client/operator.c src/client/log.c src/client/parser.c src/host.c src/crypto.c"
cli_links="$enet_links $sodium_links $gtk_links"

gcc -Wall -g $file_links $serv_sources -o server $serv_links $sodium_lib_path
gcc -Wall -g $file_links $cli_sources -o client $cli_links $sodium_lib_path
