
#ifndef _COMM_H_
#define _COMM_H_

// this should be >= RSA_KEY_LEN
#define STD_MSG_LEN (1024)
#define MAX_MSG_LEN (STD_MSG_LEN * 4)

int receive_message(const int socket, char* msg, const unsigned msg_len);

int receive_encrypted_message(const int socket, char* msg, const unsigned msg_len, const rsa_key_t privkey);

int send_message(const int socket, const char* msg, const unsigned msg_len);

int send_encrypted_message(const int socket, const char* msg, const unsigned msg_len, const rsa_key_t pubkey);


#endif

