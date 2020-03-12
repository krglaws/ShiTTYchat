
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{

  char buffer[1024];
  char* source = "SCCP: 0.1\nConnection-Request: In-Socket\n";

  char* needle = "Connection-Request: ";
  char* field_ptr = strstr(source, needle);

  sscanf(field_ptr, "Connection-Request: %s\n", buffer);

  printf("%s\n", buffer);
}

