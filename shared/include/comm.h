
#ifndef _COMM_H_
#define _COMM_H_

/* ---------------- General Options --------------- */

// this should be >= RSA_KEY_LEN
//#define STD_MSG_LEN (1024)
//#define MAX_MSG_LEN (STD_MSG_LEN * 4)


/* ------ Field String Container Sizes ----- */
#define BASE_FLD_SZ	(4)
#define EXP_FLD_SZ 	(64)
#define DIV_FLD_SZ 	(1024)
#define UNAME_FLD_SZ	(UNAMELEN + 1)


/* ------ Field Format Strings ----- */
#define BASE_FMT	"BASE: %4s"
#define EXP_FMT		"EXP: %64s"
#define DIV_FMT		"DIV: %1024s"
#define UNAME_FMT	"UNAME: %16s"


int receive_message(const int socket, char* msg, const unsigned msg_len);

int receive_encrypted_message(const int socket, char* msg, const unsigned msg_len, const rsa_key_t privkey);

int send_message(const int socket, const char* msg, const unsigned msg_len);

int send_encrypted_message(const int socket, const char* msg, const unsigned msg_len, const rsa_key_t pubkey);


#endif

