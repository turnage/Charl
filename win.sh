file_links="-Isrc"
enet_links="-lenet -lwinmm -lws2_32"
sodium_links="-lsodium"
gtk_links="`pkg-config --cflags --libs gtk+-2.0`"

serv_sources="src/server/main.c src/charl.c src/server/client.c src/server/parser.c src/host.c src/crypto.c src/win.c"
serv_links="$enet_links $sodium_links"

cli_sources="src/client/main.c src/charl.c src/client/gui.c src/client/operator.c src/client/parser.c src/client/log.c src/host.c src/crypto.c src/win.c"
cli_links="$enet_links $sodium_links `pkg-config --cflags --libs gtk+-2.0`"

gcc -Wall -g $file_links $serv_sources -o server $serv_links
gcc -Wall -g $file_links $cli_sources -o client $cli_links -mwindows