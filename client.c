
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <rsa.h>
#include <comm.h>

#define STACK_SIZE (1024*1024)

int main()
{
  int sock;
  rsa_key_t pubkey, privkey;
  rsa_init(pubkey, privkey, 1024, 62);

  printf("public exponent = %s\n", pubkey->e);
  printf("public divisor = %s\n", pubkey->d);

  // specify server address
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(421);
  server_address.sin_addr.s_addr = inet_addr("0.0.0.0");

  // create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("main(): failed to create socket");
    exit(EXIT_FAILURE);
  }

  // connect to server
  if (connect(sock, (struct sockaddr *) &server_address, sizeof(struct sockaddr)) == -1)
  {
    perror("main(): failed to connect to server address");
    exit(EXIT_FAILURE);
  }

  char* template = "UNAME: user1\nBASE: 62\nEXP: %s\nDIV: %s\n";
  char handshake[MAX_MSG_LEN];
  sprintf(handshake, template, pubkey->e, pubkey->d);

  // send handshake to server
  send_message(sock, handshake, strlen(handshake));
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(sock, &fds);

  // await server response
  if (select(sock+1, &fds, NULL, NULL, NULL) == -1)
  {
    perror("main(): failed on call to select()");
    exit(EXIT_FAILURE);
  }

  // get server response
  if (receive_encrypted_message(sock, handshake, MAX_MSG_LEN, privkey) == -1)
  {
    fprintf(stderr, "main(): failed on call to receive_encrypted_message()\n");
    exit(EXIT_FAILURE);
  }

  printf("Server response:\n%s\n", handshake);

  rsa_key_t servkey;

  char* field_ptr = strstr(handshake, "BASE: ");
  char parse_buff[2048];
  sscanf(field_ptr, "BASE: %d\n", &servkey->b);

  field_ptr = strstr(handshake, "EXP: ");
  sscanf(field_ptr, "EXP: %s\n", parse_buff);
  servkey->e = calloc(1, strlen(parse_buff) + 1);
  memcpy(servkey->e, parse_buff, strlen(parse_buff));

  field_ptr = strstr(handshake, "DIV: ");
  sscanf(field_ptr, "DIV: %s\n", parse_buff);
  servkey->d = calloc(1, strlen(parse_buff) + 1);
  memcpy(servkey->d, parse_buff, strlen(parse_buff));

  char* dis = "DISCONNECT\n";
  send_encrypted_message(sock, dis, strlen(dis), servkey);

  receive_message(sock, parse_buff, 2048);

  close(sock);

  rsa_clear_key(pubkey);
  rsa_clear_key(privkey);
}
/*
  // set up listening thread
  int pid;
  uint8_t *stack = malloc(STACK_SIZE);
  if ((pid = clone(&handle_incoming, stack+STACK_SIZE, CLONE_FILES | SIGCHLD)) == -1)
  {
    perror("main(): failed to create child");
    exit(EXIT_FAILURE);
  }

  char msg[MAX_MSG_LEN];
  while (1)
  {
    scanf("%1000s\n", msg);
    if (send_encrypted_message(socket, msg, strlen(msg), server_key) == -1)
    {
      perror("main(): failed to on call to send_encrypted_message()");
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}


void handle_incoming(int socket, rsa_key_t privkey)
{
  fd_set fds;
  char msg[MAX_MSG_LEN];

  while (1)
  {
    // clear socket set
    FD_ZERO(fds);

    // add our only socket to set
    FD_SET(socket, fds);

    // await incoming messages
    if (select(socket+1, &fds, NULL, NULL, NULL) == -1)
    {
      perror("handle_incoming(): failed on call to select()");
      exit(EXIT_FAILURE);
    }

    int len = receive_encrypted_message(socket, msg, MAX_MSG_LEN, privkey)

    if (len == -1)
    {
      fprintf(stderr, "handle_incoming(): failed on call to receive_encrypted_message()\n");
      exit(EXIT_FAILURE);
    }

    if (len == 0)
    {
      fprintf(stderr, "handle_incoming(): server disconnected\n");
      exit(EXIT_FAILURE);
    }

    printf("%s", msg);
  }
}


*/
