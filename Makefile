
CFLAGS := -Ishared/include -Iserver/include -Iclient/include -lgmp -lncurses -g 

SHARED := shared/src/rsa.c shared/src/comm.c

CLIENT := client/src/client.c client/src/ui.c client/src/init.c

SERVER := server/src/server.c server/src/client_manager.c


default: all

.PHONY: client
client: $(CLIENT) $(SHARED)
	gcc $^ $(CFLAGS) -o client.out

.PHONY: server
server: $(SERVER) $(SHARED)
	gcc $^ $(CFLAGS) -o server.out

.PHONY: all
all: client server

.PHONY: clean
clean:
	@if [ -f client.out ]; then rm -f client.out; fi;
	@if [ -f server.out ]; then rm -f server.out; fi;

