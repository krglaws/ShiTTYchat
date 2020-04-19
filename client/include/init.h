
#ifndef _INIT_H_
#define _INIT_H_

#include <stdbool.h>
#include <rsa.h>

bool valid_uname(char *uname);

bool valid_ip(char *ip);

bool valid_port(char *port);

void usage(char* name);

int connect_to_server(char *ip, char *uname, char *port);

int handshake(rsa_t pubkey, rsa_t privkey, rsa_t servkey);

#endif

