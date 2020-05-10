
#ifndef _CLIENT_H_
#define _CLIENT_H_

int client_loop(int sock, rsa_key_t privkey, rsa_key_t servkey);

int incoming_message_handler(void* args);

void sigchld_handler(int signum);

#endif

