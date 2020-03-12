
#ifndef _COMM_H_
#define _COMM_H_


int receive_message(int socket, char* msg, unsigned msg_len);

int receive_encrypted_message(int socket, char* msg, unsigned msg_len, rsa_key_t privkey);

void send_message(int socket, char* msg, unsigned msg_len);

void send_encrypted_message(int socket, char* msg, unsigned msg_len, rsa_key_t pubkey);


#endif

