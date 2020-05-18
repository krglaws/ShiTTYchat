
#ifndef _CLIENT_MANAGER_H_
#define _CLIENT_MANAGER_H_

#define ACCEPT_RESP_TMPLT "BASE: %d\nEXP: %s\nDIV: %s\n"

// client entry linked list definition
typedef struct __client_entry_struct client_entry_t;

struct __client_entry_struct
{
  int socket;
  char ip[20];
  int missed_beats;
  char uname[UNAMELEN + 1]; // add one for NULL terminator
  rsa_key_t key;
  client_entry_t* next_entry;
};


/*
 * intializes client_manager. what that really entails is just inserting the server's info
 * into the beginning of the client_list. That way it's socket number is present when get_active_fd()
 * is called (it checks all sockets, including sever socket for incoming connections).
 */
void init_client_manager(const int server_socket);


/*
 * broadcasts a message to all connected clients
 */
static int broadcast(const char* uname, const char* msg);


/*
 * handles incoming messages from logged in clients
 */
int handle_client_message(const int socket, const rsa_key_t privkey);


/*
 * validates usernames
 */
static int validate_uname(const char* uname);


/*
 * processes connection requests from new clients
 */
int new_connection(const int socket, const char* ip, rsa_key_t pubkey);


/*
 * when a client disconnects from the server (i.e. it sends a message with 0 bytes on either socket)
 * this procedure is called to remove the client from client_list.
 */
static int remove_client(const int socket);


/*
 * initializes the set of active sockets
 */
int initialize_fdset(fd_set* fds);


/*
 * retrieves the currently active socket from the socket set
 */
int get_active_fd(fd_set* fds);


/*
 * this is periodically called to check the pulse of each client
 */
void heartbeat(int sig);

#endif

