
#ifndef _RSA_H_
#define _RSA_H_


// RSA key type definition
typedef struct
{
  unsigned b; // base
  char *d; // divisor
  char *e; // exponent
} __rsa_key_struct;

typedef __rsa_key_struct rsa_key_t[1];


/* 
  Initializes a public and a private key for use
  in rsa_encrypt() and rsa_decrypt().

  param pub - public key
  param priv - private key
  param keylen - the approximate length in bits of the keys
  param base - base used to encode the keys (2 >= base <= 62)
  returns int, 0 on success, -1 on failure.

  NB: This procedure may fail for one of two reasons:
      1. The file '/dev/urandom' could not be opened, or
      2. An inverse of the public exponent does not exist (unlikely).
*/
int rsa_init(rsa_key_t pub, rsa_key_t priv, const unsigned keylen, const unsigned base);


/*
  Frees up allocated memory in an initialized key.

  param key - the key
 */
void rsa_clear_key(rsa_key_t key);


/* 
  Encrypts a character and stores the encrypted value
  into a char*.

  param enc - buffer in which to store encrypted data
  param count - the number of bytes in raw to encrypt
  param raw - buffer containing data to be encrypted
  param pub - public key
  returns int, the length in bytes of the encrypted data

  NB: Count must be less than the length of the rsa divisor
      (in bytes). Otherwise the result will not be able to be
      decrypted.
*/
int rsa_encrypt(char* enc, const unsigned count, const char* raw, const rsa_key_t pub);


/*
  Decrypts a string and returns the result as a char.

  param raw - buffer in which to store decrypted data
  param count - the number of bytes in enc to decrypt
  param enc - buffer containing data to be decrypted
  param priv - private key
  returns int, the length in bytes of the decrypted data

*/
int rsa_decrypt(char* raw, const unsigned count, const char* enc, const rsa_key_t priv);


/*
  Returns the (approximate) maximum number of bytes that an initialized public key can encrypt.

  param key - the key
 */
unsigned rsa_max_bytes(const rsa_key_t key);


#endif

