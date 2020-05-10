
gcc src/server.c ../shared/src/comm.c ../shared/src/rsa.c src/client_manager.c -Iinclude -I../shared/include -lncurses -lgmp -g -o server.out

