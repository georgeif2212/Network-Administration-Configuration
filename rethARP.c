#include "eth.h"
#include <unistd.h>
// #define MAC_STRING_SIZE 18

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Error en argumentos.\n\n");
    printf("reth INTERFACE-ENTRADA\n\n");
    exit(1);
  }
  int myIdentifier = 99;
  char identifierString[2];
  sprintf(identifierString, "%d", myIdentifier);
  listener(argv[1], myIdentifier);

  return (0);
}
