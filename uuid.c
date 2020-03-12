
#include <stdlib.h>
#include <uuid.h> 

static const char charlist[26] = "abcdefghijklmnopqrstuvwxyz";

int generate_uuid(char* uuid_container)
{
  FILE* urandom = fopen("/dev/urandom", "r");

  if (urandom == NULL)
  {
    perror("generate_uuid()");
    return -1;
  }

  for (int i = 0; i < UUID_LEN-1; i++)
  {
    char r;
    fread(&r, 1, 1, urandom);
    uuid_container[i] = charlis[r % 26];
  }
  return 0;
}

