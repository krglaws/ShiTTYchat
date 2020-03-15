
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
    if (ioctl(socket, FIONREAD, &available) == -1)
    {
      perror(stderr, "receive_message()");
      return -1;
    }

    // if there are too many bytes, stop to prevent buffer overflow
    if (total + available > msg_len)
    {
      fprintf(stderr, "receive_message(): incoming message is too long.\n");
      return -1;
    }

  // when recv returns 0, remote is done sending
  } while (received > 0);

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
  {
    fprintf(stderr, "receive_encrypted_message(): failed on call to receive_message()\n");
    return -1;
  }

  // could validate here, or let the message parser figure out if it's gibberish

  // Decrypt message and return length
  return rsa_decrypt(msg, msg, privkey);
}


int send_message(int socket, char* msg, unsigned msg_len)
{
  // Keep sending while total bytes sent is less than message length
  int total = 0, sent = 0;
  while(total < msg_len)
  {
    if ((sent = send(socket, msg + total, msg_len - total, 0)) == -1)
    {
      perror("send_message()");
      return -1;
    }
    total += sent;
  }

  return 0;
}


int send_encrypted_message(int socket, char* msg, unsigned msg_len, rsa_key_t pubkey)
{
  char enc[MAX_MSG_LEN];
  char* enc_next = enc;
  char* msg_end = msg + msg_len;

  // Get max bytes for encryption
  int max_bytes = rsa_max_bytes(pubkey);

  // while end of message not reached
  while (msg < end)
  {
    // Assume we are sending the maximum length possible
    int num_bytes = max_bytes;

    // Truncate num_bytes if > remaining to be sent
    if (msg + num_bytes > end)
      num_bytes = end - msg;

    // check if we are out of space in encryption buffer
    if (enc_next + max_bytes > MAX_MSG_LEN)
    {
      fprintf(stderr, "send_encrypted_message(): message too long (> %d)\n", MAX_MSG_LEN);
      return -1;
    }

    // Encrypt and save the length of encrypted result
    int enc_len = rsa_encrypt(enc_next, num_bytes, msg, pubkey);

    enc_next += enc_len;

    // shift up to the next segment
    msg += num_bytes;
  }

  // send encrypted bytes
  if (send_message(socket, enc, enc_next - enc) == -1)
  {
    sprintf(stderr, "send_encrypted_message(): failed on call to send_message()\n");
    return -1;
  }

  return 0;
}

