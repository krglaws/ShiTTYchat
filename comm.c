
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <rsa.h>


int receive_message(int socket, char* msg, unsigned msg_len)
{
  int total = 0, received = 0;

  do
  {
    // read from socket
    received = recv(socket, msg + total, msg_len, 0);
    if (received == -1)
    {
      perror("receive_message()");
      exit(EXIT_FAILURE);
    }

    // add received bytes to total
    total += received;

    // check if there are bytes still to be read
    int available = 0;
    ioctl(socket, FIONREAD, &available);

    // if there are too many bytes, stop to prevent buffer overflow
    if (total + available > msg_len)
    {
      fprintf(stderr, "receive_message(): incoming message is too long.\n");
      return -1;
    }

  // when recv returns 0, remote is done sending
  } while(received > 0);

  // check for empty message
  if (total == 0)
  {
    fprintf(stderr, "receive_message(): message empty.\n");
    return -1;
  }

  return total;
}


int receive_encrypted_message(int socket, char* msg, unsigned msg_len, rsa_key_t privkey)
{
  // Read raw message bytes into msg
  if (receive_message(socket, msg, msg_len) == -1)
    return -1;

  // could validate here, or let the message parser figure out if it's gibberish

  // Decrypt message and return length
  return rsa_decrypt(msg, msg, privkey);
}


void send_message(int socket, char* msg, unsigned msg_len)
{
  // Keep sending while total bytes sent is less than message length
  int total = 0, sent = 0;
  while(total < msg_len)
  {
    if ((sent = send(socket, msg + total, msg_len - total, 0)) == -1)
    {
      perror("send_message()");
      exit(EXIT_FAILURE);
    }
    total += sent;
  }
}


void send_encrypted_message(int socket, char* msg, unsigned msg_len, rsa_key_t pubkey)
{
  // This will be used to encrypt and send in segments
  char *start = msg;

  /* The length of the modulus is the maximum possible length of
     encrypted value */
  int enc_len = strlen(pubkey->m);
  char enc[enc_len + 1]; // add one for NULL terminator

  // Get the maximum number of bytes that can be encrypted at a time
  int max_bytes = rsa_max_bytes(pubkey);

  // while start has not reached the end of the message
  while(start < msg + msg_len)
  {
    // Assume we are sending the maximum length possible
    int num_bytes = max_bytes;

    // Truncate num_bytes if > remaining to be sent
    if (msg_len - msg < max_bytes)
      num_bytes = msg_len - msg;

    // Encrypt and save the length of encrypted result
    int bytes_to_send = rsa_encrypt(enc, num_bytes, message, pubkey);

    // Send bytes
    send_message(socket, enc, bytes_to_send);

    // shift up to the next segment
    start += num_bytes;
  }
}




