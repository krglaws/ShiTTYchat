
#include <server.h>
#include <client_manager.h>

static unsigned num_clients = 0;

static client_entry_t* client_list = NULL;


static int validate_connection_request(char msg_buffer, unsigned len);


static int hand_shake(const int socket, const rsa_key_t server_key, rsa_key_t client_key)
{
  char *valid_out_request = "{ShiTTYchat-Version: " VERSION "; Connection-Request: OUT-sock}";
  char *valid_in_request =  "{ShiTTYchat-Version: " VERSION "; Connection-Request: IN-sock}";

  int len = strlen(valid_out_request);
  char msg_buffer[len+1];
  memset(msg_buffer, 0, len+1);

  char* reject_msg = "{Connection-Request: rejected; Reason: message too long?}"
  // we expect a short message from client with version info and connection type
  if ((len = receive_message(socket, msg_buffer, len)) == -1)
  {
    send_message(socket, reject_msg, strlen(reject_msg));
    fprintf(stderr, "exchange_keys(): invalid connection request: call to receive_message() failed.\n");
    return -1;
  }

  char* accept_msg = "{Connection-Request: accepted}";
  // compare request with valid OUT-sock request
  if (strcmp(valid_out_request, msg_buffer) == 0)
  {
    // tell client request is accepted
    send_message(socket, accept_msg, strlen(accept_msg));
  }

  // compare request with valid IN-sock request
  else if (strcmp(valid_in_request, msg_buffer) == 0)
  {
    
  }

  fprintf(stderr, "exchange_keys(): invalid connection request message.\n");
  return -1; 
}


/* this procedure is called only
   once at the start of the program */
void init_client_manager(int server_socket)
{
  // create client_entry_t to hold server socket at very beginning of list
  client_list = malloc(sizeof(client_entry_t));
  char ip[IPSTRLEN] = "0.0.0.0";
  char uname[UNAMELEN] = "SERVER";

  // fill in full server entry except rsa key, no need for it to be here
  client_list->socket = server_socket;
  memcpy(client_list->ip, ip, IPSTRLEN);
  memcpy(client_list->uname, uname, UNAMELEN);
}


int add_client(const int socket, const char* ip, const rsa_key_t server_key)
{
  // check if we are at max capacity
  if (num_clients == MAXCLIENTS)
  {
    fprintf(stderr, "add_client(): at max client capacity\n");
    return -1;
  }

  char uname[UNAMELEN];
  rsa_key_t client_key;

  // try to exchange keys. if something goes wrong, let exchange_keys() report it.
  if (hand_shake(socket, server_key, client_key, uname) == -1)
  {
    fprintf(stderr, "add_client(): failed on call to hand_shake()\n");
    return -1;
  }

  // allocate space for new client entry
  client_entry_t* entry = malloc(sizeof(client_entry_t));

  // fill in entry with client attributes
  entry->socket = socket;
  memcpy(entry->ip, ip, strlen(ip));
  memcpy(entry->uname, uname, UNAMELEN);

  entry->key->m = client_key->m;
  entry->key->e = client_key->e;
  entry->key->b = client_key->b;

  entry->next_entry = NULL;

  // find end of client entry list and append new client entry
  client_entry_t* iterator = client_list;
  while (iterator->next_entry != NULL)
    iterator = iterator->next_entry;

  iterator->next_entry = entry;

  return 0;
}


int remove_client(int socket)
{
  if (client_list == NULL)
  {
    fprintf(stderr, "remove_client(): client list is empty\n");
    return -1;
  }

  client_entry_t* iterator = client_list;

  // check if first entry is the one to be removed
  if (client_list->socket == socket)
  {
    // skip over first entry
    client_list = client_list->next_entry;

    // then free it up
    rsa_clear_key(iterator->key);
    free(iterator);
  }

  // look for entry with matching socket number
  while (iterator->next_entry != NULL && 
         iterator->next_entry->socket != socket)
    iterator = iterator->next_entry;

  // check if we have reached the end
  if (iterator->next_entry == NULL)
  {
    fprintf(stderr, "remove_client(): no connection with socket %d exists in client list\n", socket);
    return -1;
  }

  // save next entry, set pointer to next entry to next next entry
  client_entry_t* deletethis = iterator->next_entry;
  iterator->next_entry = deletethis->next_entry;

  // free up old entry
  rsa_clear_key(deletethis->key);
  free(deletethis);

  return 0;
}


int initialize_fdset(fd_set* fds)
{
  int max = 0;

  // zero out socket set
  FD_ZERO(fds);

  /* iterate over each client entry, adding each socket to socket set,
     and also keeping track of highest socket value */
  client_entry_t* iterator = client_list;
  while (iterator)
  {
    // get socket from client entry
    int socket = iterator->socket;

    // add socket to set
    FD_SET(socket, fds);

    // check if current socket is the highest we've seen
    if (socket > max) max = socket;

    // get next client entry
    iterator = iterator->next_entry;
  }

  return max; 
}


int get_active_fd(fd_set* fds)
{
  // iterate over client entry list and find active socket
  client_entry_t* iterator = client_list;
  while (iterator)
  {
    int socket = iterator->socket;
    if (FD_ISSET(socket, fds))
      return socket;
  }

  fprintf(stderr, "get_active_fd(): could not find active socket\n");
  return -1;
}

