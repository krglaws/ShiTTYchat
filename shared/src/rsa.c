
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <gmp.h>

#include <rsa.h>


/* Called by rsa_init(). Generates a random
   prime number of a specified bit length */
static void rand_prime(mpz_t p, unsigned bits, FILE *randfile)
{
  // # of bits in most significant byte
  int sig_bits = bits % 8;

  // total number of bytes
  int num_bytes = (bits / 8) + (sig_bits > 0);

  // Get most significant byte
  uint8_t byte;
  fread(&byte, 1, 1, randfile);

  if (sig_bits)
    byte = byte >> (8 - sig_bits);

  // add byte to p, and bit shift 8 bits to the left
  mpz_add_ui(p, p, byte);

  // Get remaining bytes
  for (int i = 1; i < num_bytes; i++)
  {
    fread(&byte, 1, 1, randfile);
    mpz_mul_ui(p, p, 256);
    mpz_add_ui(p, p, byte);
  }

  // find next prime above p
  mpz_nextprime(p, p);
}


/* Initializes a public and a private key for use
   in rsa_encrypt() and rsa_decrypt(). */
int rsa_init(rsa_key_t pub, rsa_key_t priv, const unsigned keylen, const unsigned base)
{
  FILE *urandom;
  mpz_t P, Q, N, L, E, D, M;

  // Open '/dev/urandom' for secure random number generation.
  if (NULL == (urandom = fopen("/dev/urandom", "r")))
    return -1;

  // Initialize mpz_t's
  mpz_inits(P, Q, N, L, E, D, NULL);

  // Generate two random primes
  rand_prime(P, keylen/2, urandom);
  rand_prime(Q, keylen/2, urandom);

  fclose(urandom);

  // Multiply them to create divisor
  mpz_mul(N, P, Q);

  // Calculate lcm(P-1, Q-1)
  mpz_sub_ui(P, P, 1);
  mpz_sub_ui(Q, Q, 1);
  mpz_lcm(L, P, Q);

  // Set public exponent
  mpz_set_ui(E, 65537);

  /* Generate private exponent. If mpz_invert() returns '0',
     an inverse value could not be found, return error */
  int stat = 0 != mpz_invert(D, E, L);

  // set key base encoding
  pub->b = base;
  priv->b = base;

  // Allocate key space
  int len = mpz_sizeinbase(E, base);
  pub->e = malloc(len + 1);

  len = mpz_sizeinbase(D, base);
  priv->e = malloc(len + 1);

  len = mpz_sizeinbase(N, base);
  pub->d = malloc(len + 1);
  priv->d = malloc(len + 1);

  // Convert E, D, and N to char* and store into keys.
  mpz_get_str(pub->e, base, E);
  mpz_get_str(pub->d, base, N);
  mpz_get_str(priv->e, base, D);
  mpz_get_str(priv->d, base, N);

  // Free up mpz_t's
  mpz_clears(P, Q, N, L, E, D, NULL);

  return stat;
}


/* frees up an initialized key */
void rsa_clear_key(rsa_key_t key)
{
  free(key->e);
  key->e = NULL;

  free(key->d);
  key->d = NULL;
}


/* Encrypts 'count' bytes from char* raw and stores the encrypted value
   into char* enc */
int rsa_encrypt(char* enc, const unsigned count, const char* raw, const rsa_key_t pub)
{
  // Declare and initialize mpz_t's
  mpz_t msg, div, exp;
  mpz_inits(msg, div, exp, NULL);

  // Convert raw data into mpz_t
  mpz_add_ui(msg, msg, raw[0]);
  for (int i = 1; i < count; i++)
  {
    mpz_mul_ui(msg, msg, 256); // same as msg <<= 8
    mpz_add_ui(msg, msg, raw[i]);
  }

  // Set div and exp to public divisor and exponent
  mpz_set_str(div, pub->d, pub->b);
  mpz_set_str(exp, pub->e, pub->b);

  // Encrypt and store into enc buffer
  mpz_powm(msg, msg, exp, div);
  mpz_get_str(enc, pub->b, msg);

  // free up mpz_t's
  mpz_clears(msg, div, exp, NULL);

  // return enc length
  return strlen(enc);
}


/* Decrypts 'count' bytes from enc and stores the result into raw. */
int rsa_decrypt(char* raw, const unsigned count, const char* enc, const rsa_key_t priv)
{
  /* mzp_set_str() will use all bytes until
     null terminator is reached,
     so we must separate the 'chunk' being
     decrypted into a separate buffer. */
  char temp[count+1];
  memcpy(temp, enc, count);
  temp[count] = 0; // NULL terminator

  // Declare and initialize mpz_t's
  mpz_t msg, div, exp, rem;
  mpz_inits(msg, div, exp, rem, NULL);

  // Set msg to chunk value
  mpz_set_str(msg, temp, priv->b);

  // Set div and exp to private divisor and exponent
  mpz_set_str(div, priv->d, priv->b);
  mpz_set_str(exp, priv->e, priv->b);

  // Decrypt
  mpz_powm(msg, msg, exp, div);

  // Store decrypted bytes into raw
  int i = 0;
  while (mpz_cmp_ui(msg, 0) != 0)
  {
    mpz_tdiv_qr_ui(msg, rem, msg, 256);
    raw[i++] = mpz_get_ui(rem);
  }
  raw[i] = 0;

  // Reverse decrypted bytes
  char* end = raw + i - 1;
  while (raw < end)
  {
    *raw ^= *end;
    *end ^= *raw;
    *raw ^= *end;

    raw++;
    end--;
  }

  // free up mpz_t's
  mpz_clears(msg, div, exp, rem, NULL);

  // return raw length
  return i;
}


// returns approximate number of bytes a key can encrypt
unsigned rsa_max_bytes(const rsa_key_t key)
{
  mpz_t num;
  mpz_init(num);
  mpz_set_str(num, key->d, key->b);

  int bits = mpz_sizeinbase(num, 2);

  mpz_clear(num);
  return (bits / 8) - 1;
}

