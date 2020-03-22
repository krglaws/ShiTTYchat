
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <rsa.h>

#include <comm.h>


int receive_message(const int socket, char* msg, const unsigned msg_len)
{
  int total = 0, received = 0, available = 0;

  do
  {
    // read from socket
    received = recv(socket, msg + total, msg_len, 0);
    if (received == -1)
    {
      perror("receive_message()");
      return -1;
    }

    // add received bytes to total
    total += received;

    // check if there are bytes still to be read
    if (ioctl(socket, FIONREAD, &available) == -1)
    {
      perror("receive_message()");
      return -1;
    }

    // if there are too many bytes, stop to prevent buffer overflow
    if (total + available > msg_len)
    {
      fprintf(stderr, "receive_message(): incoming message is too long.\n");
      return -1;
    }

  // loop while bytes are available
  } while (available > 0);

  return total;
}


/* 
 * When receiving and decrypting a message, the length of the encrypted
 * message must be aligned with the 'chunk size' of the rsa key. That is,
 * received_bytes % chunk_size must be 0.
 */
int receive_encrypted_message(const int socket, char* msg, const unsigned msg_len, const rsa_key_t privkey)
{
  // temp buffer to hold encrypted message
  char enc[MAX_MSG_LEN];
  char* enc_next = enc;

  int chunk_size = strlen(privkey->d);

  // read in message from socket
  int received = receive_message(socket, enc, MAX_MSG_LEN);

  // find end of encrypted message
  char* enc_end = enc + received;

  // check if receive_message() failed
  if (received == -1)
  {
    fprintf(stderr, "receive_encrypted_message(): failed on call to receive_message()\n");
    return -1;
  }

  // check if encrypted message is not aligned with chunk size
  if (received % chunk_size != 0)
  {
    fprintf(stderr, "receive_encrypted_message(): encrypted message is not aligned with chunk size\n");
    return -1;
  }

  // if message is empty, just return
  if (received == 0)
    return 0;

  char* msg_next = msg;
  char* msg_end = msg + msg_len;

  // temp buffer to hold decrypted chunk
  char raw_chunk[chunk_size + 1]; // +1 for NULL terminator
  int raw_len = 0;

  while (enc_next < enc_end)
  {
    // decypt, store into raw_chunk
    int dec_bytes = rsa_decrypt(raw_chunk, chunk_size, enc_next, privkey);

    raw_len += dec_bytes;

    // check if raw message has exceeded msg_len
    if (raw_len > msg_len)
    {
      fprintf(stderr, "receive_encrypted_message(): decrypted message is too long\n");
      return -1;
    }

    // increment to decrypt next chunk
    enc_next += chunk_size;

    // copy decrypted bytes into msg
    memcpy(msg_next, raw_chunk, dec_bytes);

    // increment to end of decrypted bytes
    msg_next += dec_bytes;
  }

  // set null terminator
  msg_next[0] = 0;

  return raw_len;
}


int send_message(const int socket, const char* msg, const unsigned msg_len)
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


/*
 * When we encrypt a message, it must be carefully sectioned into "chunks", each
 * chunk being a segment of bytes of strict length. This way, an entire message
 * can be sent at once without accidentally trying to decrypt, say, the second half
 * of one chunk and the first half of its neighbor (which would of course result
 * in gibberish).
 *
 * The length of each chunk is determined by the length of the rsa key divisor in
 * bytes. If an encrypted value happens to be shorter in length than that, it must
 * be prepended with '0' characters so that it is the correct length when sent over
 * the network.
 */
int send_encrypted_message(const int socket, const char* msg, const unsigned msg_len, const rsa_key_t pubkey)
{
  // chunk length == length of divisor in bytes 
  int chunk_len = strlen(pubkey->d);

  // max encryptable bytes (one or two bytes less than chunk_length)
  int max_bytes = rsa_max_bytes(pubkey);

  // find number of chunks required
  int num_chunks = msg_len / max_bytes;
  num_chunks += msg_len % chunk_len ? 1 : 0;

  // check if msg is too long
  if (num_chunks * chunk_len > MAX_MSG_LEN)
  {
    fprintf(stderr, "send_encrypted_message(): message too long\n");
    return -1;
  }

  // encryption buffer
  char enc[MAX_MSG_LEN + 1];

  // keep track of where next bytes should be stored
  char* enc_next = enc;
  char* enc_end = enc + MAX_MSG_LEN;

  // temporary encryption buffer
  char enc_tmp[chunk_len + 1];

  // keep track of next bytes to encrypt
  char* msg_next = (char*) msg;
  char* msg_end = (char*) msg + msg_len;

  // while there are bytes left to encrypt
  while (msg_next < msg_end)
  {
    // assume we are encrypting the maximum number possible
    int bytes_to_encrypt = max_bytes;
    if (bytes_to_encrypt > msg_end - msg_next)
      bytes_to_encrypt = msg_end - msg_next;

    // encrypt and store into enc_tmp
    int enc_bytes = rsa_encrypt(enc_tmp, bytes_to_encrypt, msg_next, pubkey);

    // if the number of encrypted bytes is too small, extend with zeros
    if (enc_bytes < chunk_len)
    {
      char* zero_to_here = enc_next + (chunk_len - enc_bytes);
      while (enc_next < zero_to_here)
      {
        *enc_next = '0';
        enc_next++;
      }
    }

    // copy enc_tmp into enc, then increment buffers
    memcpy(enc_next, enc_tmp, enc_bytes);
    enc_next += enc_bytes;
    msg_next += bytes_to_encrypt;
  }

  // send encrypted message
  if (send_message(socket, enc, enc_next - enc) == -1)
  {
    fprintf(stderr, "send_encrypted_message(): failed on call to send_message()\n");
    return -1;
  }

  return 0;
}

