#include "eth.h"
#include <unistd.h>
// #define MAC_STRING_SIZE 18

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Error en argumentos.\n\n");
    printf("reth INTERFACE-ENTRADA MY-IDENTIFIER\n\n");
    exit(1);
  }
  int myIdentifier = atoi(argv[2]);
  listener(argv[1], myIdentifier);

  return (0);
}
