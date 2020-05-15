
#ifndef _SERVER_H_
#define _SERVER_H_

// the maximum number of clients allowed to connect
#define MAXCLIENTS 10

// maximum user name length
#define UNAMELEN 16

#define RSA_KEY_LEN 1024

#define RSA_ENCODING 62

#define DEFAULT_PORT (42069)

void usage(char *name);

bool valid_port(char *port);

int server(char *port_str);

#endif

