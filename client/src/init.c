
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#include <rsa.h>
#include <comm.h>
#include <client.h>
#include <init.h>

#include <settings.h>


int main(int argc, char **argv)
{
  char *uname = NULL;
  char *ip = NULL;
  char *port = NULL;

  /* get args */
  int c;
  while ((c = getopt(argc, argv, "u:i:p:")) != -1)
  {
    switch (c)
    {
    case 'u':
      uname = optarg;
      break;
    case 'i':
      ip = optarg;
      break;
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

  /* check if essential args are NULL */
  if (!(uname && ip))
  {
    usage(argv[0]);
    return -1;
  }

  /* validate args */
  if (!valid_uname(uname) || !valid_ip(ip) || !valid_port(port))
  {
    return -1;
  }

  /* generate rsa keys */
  rsa_key_t pubkey, privkey, servkey;
  rsa_init(pubkey, privkey, RSAKEYLEN, RSAKEYENC);

  /* connect to server */
  int sock;
  if ((sock = connect_to_server(ip, uname, port)) == -1)
  {
    fprintf(stderr, "main(): failed on call to connect_to_server()\n");
    rsa_clear_key(pubkey);
    rsa_clear_key(privkey);
    return -1;
  }

  /* perform handshake with server */
  if (handshake(sock, uname, pubkey, privkey, servkey) == -1)
  {
    fprintf(stderr, "main(): failed on call to handshake()\n");
    rsa_clear_key(pubkey);
    rsa_clear_key(privkey);
    return -1;
  }
  rsa_clear_key(pubkey);

  /* start client (never returns) */
  client_loop(sock, privkey, servkey);
}


bool valid_uname(char *uname)
{
  int len = strlen(uname);
  if (len > UNAMELEN)
  {
    fprintf(stderr, "user name too long (%d characters maximum)\n", UNAMELEN);
    return false;
  }

  for (int i = 0; i < len; i++)
  {
    char curr = uname[i];
    if (!isprint(curr) || isspace(curr) || ispunct(curr))
    {
      fprintf(stderr, "user name contains invalid characters\n");
      return false;
    }
  }

  return true;
}


bool valid_ip(char *ip)
{
  struct sockaddr_in sa;

  if (!ip || inet_pton(AF_INET, ip, &(sa.sin_addr)) == 0)
  {
    fprintf(stderr, "invalid ip address\n");
    return false;
  }

  return true;
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


void usage(char *name)
{
  fprintf(stderr, "usage: %s -u <user name> -i <ip> [-p <port>]\n", name);
}


int connect_to_server(char *ip, char *uname, char *port)
{
  /* figure out port number */
  int iport = DEFAULT_PORT; // default
  if (port != NULL)
  {
    iport = strtol(port, NULL, 10);
  }

  /* set up server address */
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(iport);
  server_address.sin_addr.s_addr = inet_addr(ip);

  /* create socket */
  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("connect_to_server()");
    return -1;
  }

  /* connect to server */
  if (connect(sock, (struct sockaddr *) &server_address, sizeof(struct sockaddr)) == -1)
  {
    perror("connect_to_server()");
    return -1;
  }

  return sock;
}


int handshake(int sock, char *uname, rsa_key_t pubkey, rsa_key_t privkey, rsa_key_t servkey)
{
  /* prepare client info */
  char* template = "UNAME: %s\nBASE: %d\nEXP: %s\nDIV: %s\n";

  int bufflen = 512;
  char msgbuff[bufflen];
  memset(msgbuff, 0, bufflen);

  sprintf(msgbuff, template, uname, pubkey->b, pubkey->e, pubkey->d);

  /* send info */
  send_message(sock, msgbuff, strlen(msgbuff));

  /* get server response */
  if (receive_encrypted_message(sock, msgbuff, bufflen-1, privkey) == -1)
  {
    fprintf(stderr, "handshake(): failed on call to receive_encrypted_message()\n");
    return -1;
  }

  /* check if server is at max capacity */
  if (strcmp(msgbuff, "Server at max capacity\n") == 0)
  {
    fprintf(stderr, "handshake(): server at max capacity\n");
    return -1;
  }

  /* check if username already exists */
  if (strcmp(msgbuff, "Username already exists\n") == 0)
  {
    fprintf(stderr, "handshake(): username already exists\n");
    return -1;
  }

  /* parse server info */
  char *field_ptr;
  char parse_buff[RECVBUFFLEN];
  int len;

  if ((field_ptr = strstr(msgbuff, "BASE: ")) == NULL)
  {
    fprintf(stderr, "handshake(): server response missing BASE field\n");
    return -1;
  }
  sscanf(field_ptr, "BASE: %d\n", &servkey->b);

  if ((field_ptr = strstr(msgbuff, "EXP: ")) == NULL)
  {
    fprintf(stderr, "handshake(): server response missing EXP field\n");
    return -1;
  }
  sscanf(field_ptr, "EXP: %s\n", parse_buff);
  len = strlen(parse_buff);
  servkey->e = malloc(len + 1);
  memcpy(servkey->e, parse_buff, len);
  servkey->e[len] = '\0';

  if ((field_ptr = strstr(msgbuff, "DIV: ")) == NULL)
  {
    fprintf(stderr, "handshake(): server response missing DIV field\n");
    return -1;
  }
  sscanf(field_ptr, "DIV: %s\n", parse_buff);
  len = strlen(parse_buff);
  servkey->d = malloc(len + 1);
  memcpy(servkey->d, parse_buff, len);
  servkey->d[len] = '\0';

  return 0;
}


