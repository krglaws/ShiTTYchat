
#ifndef _CLIENT_MANAGER_H_

#ifndef _CLIENT_MANAGER_H_
#define _CLIENT_MANAGER_H_


// the maximum number of clients allowed to connect
#define MAXCLIENTS 10


/* this includes NULL terminator, so really the maximum username length is 16.
   When reading in the username from login, be sure to expect UNAMELEN-1. */
#define UNAMELEN 17

// maximum length of an IP, plus NULL terminator
#define IPSTRLEN 17

typedef struct
{
  int out_sock;
  int in_sock;
  char ip[IPSTRLEN];
  char uname[UNAMELEN];
  char uuid[UUID_LEN];
  rsa_key_t key;
  client_entry_t* next_entry;
} client_entry_t;


/*
 * handshake() steps:
 *
 * 1. client sends connection request
 *    - some way of indicating:
 *      a. that this is indeed a shittychat client trying to connect 
 *      b. whether this is the IN socket or the OUT socket
 *      example: "{ShiTTYchat-version: 0.0.1; Connection-Request: OUT-sock}"
 *
 * if this is OUT socket (which is the first one):
 *
 * 2. server validates connection request, and sends some sort of "accept" message
 * 
 * 3. client sends his public key
 *
 * -- from here on out, messages are encrypted --
 *
 * 4. server encrypts a message with client's key with the following info:
 *   - UUID to be passed back to server when client connects its IN socket
 *   - the server's public key
 *
 * 5. that's it, OUT socket is set up
 *
 * if this is IN socket (which is the second one): 
 *
 * 2. server validates connection request, sends some sort of "accept" message (plain text)
 *
 * 3. client sends message encrypted with server's key (during OUT socket setup) containing:
 *   - UUID, aaand i think thats it.
 *
 * 4. done. IN socket is setup
 */
static int handshake(int socket, rsa_key_t key);


/*
 * intializes client_manager. what that really entails is just inserting the server's info
 * into the beginning of the client_list. That way it's socket number is present when get_active_fd()
 * is called (it checks all sockets, including sever socket for incoming connections).
 */
void init_client_manager(int server_socket);


/*
 * adds a client's out_socket, ip, uname, uuid, and public key to the client_list
 */
int add_client(int socket, char* ip, rsa_key_t key);


/*
 * when a client disconnects from the server (i.e. it sends a message with 0 bytes on either socket)
 * this procedure is called to remove the client from client_list.
 */
int remove_client(int socket);


#endif

