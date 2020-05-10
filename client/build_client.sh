
gcc src/client.c src/ui.c src/init.c ../shared/src/comm.c ../shared/src/rsa.c -Iinclude -I../shared/include -lgmp -lncurses --static -o client.out -g

