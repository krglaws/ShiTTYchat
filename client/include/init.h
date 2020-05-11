
#ifndef _INIT_H_
#define _INIT_H_

bool valid_uname(char *uname);

bool valid_ip(char *ip);

bool valid_port(char *port);

void usage(char* name);

int connect_to_server(char *ip, char *uname, char *port);

int handshake(int sock, char *uname, rsa_key_t pubkey, rsa_key_t privkey, rsa_key_t servkey);

#endif

