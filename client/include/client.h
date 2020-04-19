
#ifndef _CLIENT_H_
#define _CLIENT_H_

int client_loop(int sock, rsa_t privkey, rsa_t servkey);

static int incoming_message_handler(int sock, rsa_t privkey, char* msgbuf, int initsize);

#endif

