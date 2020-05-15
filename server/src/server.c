
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <settings.h>
#include <rsa.h>
#include <server.h>
#include <client_manager.h>


int main(int argc, char** argv)
{
  char *port = NULL;

  /* get args */
  int c;
  while ((c = getopt(argc, argv, "p:")) != -1)
  {
    switch (c)
    {
    case 'p':
      port = optarg;
      break;
    case '?':
      usage(argv[0]);
      return -1;
    default:
      abort();
    }
  }

  if (optind < argc && argv[optind])
  {
    fprintf(stderr, "%s: unknown argument -- '%s'\n", argv[0], argv[optind]);
    usage(argv[0]);
    return -1;
  }

  /* validate port */
  if (port && !valid_port(port))
  {
    usage(argv[0]);
    return -1;
  }

  /* ne'er returns */
  server(port);
}


void usage(char *name)
{
  fprintf(stderr, "usage: %s [-p <port>]\n", name);
}


bool valid_port(char *port)
{
  if (port == NULL) return true;

  unsigned p = strtol(port, NULL, 10);

  if (p < 1 || p > 65535)
  {
    fprintf(stderr, "invalid port\n");
    return false;
  }

  return true;
}


int server(char *port_str)
{
  /* socket file descriptors */
  int server_fd, client_fd, active_fd, max_fd; 

  /* server info */
  struct sockaddr_in server_addr, client_addr;
  unsigned short port = DEFAULT_PORT;
  int addr_len = sizeof(struct sockaddr_in);
  char ip_str[20] = {0};
  fd_set readfds;

  if (port_str)
  {
    port = strtol(port_str, NULL, 10);
  }

  // initialize rsa keys
  rsa_key_t pubkey, privkey;
  rsa_init(pubkey, privkey, RSAKEYLEN, RSAKEYENC);

  // clear server and client addresses
  memset(&server_addr, 0, addr_len);
  memset(&client_addr, 0, addr_len);

  // set up server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, "0.0.0.0", &(server_addr.sin_addr));

  // create server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  // bind server socket to address and port
  if (bind(server_fd, (struct sockaddr*) &server_addr, addr_len) < 0)
  {
    perror("Failed to bind socket to address");
    exit(EXIT_FAILURE);
  }

  // assign server socket to listening socket
  if (listen(server_fd, 10) < 0)
  {
    perror("Failed to designate server socket");
    exit(EXIT_FAILURE);
  }

  // initialize client manager
  init_client_manager(server_fd);

  printf("Listening on port %d...\n", ntohs(server_addr.sin_port));
  while (1)
  {
    // keep track of highest socket value in socket set
    max_fd = initialize_fdset(&readfds);

    // await connections
    if (select(max_fd+1, &readfds, NULL, NULL, NULL) <= 0)
    {
      perror("failed to wait for socket activity");
      exit(EXIT_FAILURE);
    }

    // get active socket
    if ((active_fd = get_active_fd(&readfds)) == -1)
    {
      fprintf(stderr, "no active fds\n");
      exit(EXIT_FAILURE); 
    }

    // if server socket is the active socket, it means we have a new connection
    if (active_fd == server_fd)
    {
      // accept new connection
      if ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &addr_len)) < 0)
      {
        perror("Failed to accept connection");
        exit(EXIT_FAILURE);
      }

      // convert client address to printable string
      if (inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), ip_str, sizeof(ip_str)) == NULL)
      {
        perror("Failed to read client address");
        exit(EXIT_FAILURE);
      }
      printf("Connected to %s.\n", ip_str);

      // add new connection
      if (new_connection(client_fd, ip_str, pubkey) == -1)
        fprintf(stderr, "main(): call to new_connection() failed\n");
    }

    // otherwise, the active socket is already connected
    else
    {
      // handle incoming message from existing client
      if (handle_client_message(active_fd, privkey) == -1)
        fprintf(stderr, "main(): call to handle_client_message() failed\n");
    }
  }
}

