
#ifndef _CLIENT_MANAGER_H_
#define _CLIENT_MANAGER_H_


#define MAXCLIENTS 10

/* this includes NULL terminator, so really the maximum username length is 15.
   When reading in the username from login, be sure to expect UNAMELEN-1. */
#define UNAMELEN 16

#define IPSTRLEN 16

typedef struct
{
  int socket;
  char ip[IPSTRLEN];
  char uname[UNAMELEN];
  rsa_key_t key;

  client_entry_t* next_entry;
} client_entry_t;


static int exchange_keys(int socket, rsa_key_t key);

void init_client_manager();

int add_client(int socket, char* ip, rsa_key_t key);

int remove_client(int socket);


#endif

