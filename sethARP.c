#include "eth.h"

/*
códigos:
dame tu mac --- 10
te va la mac --- 20
estos son datos
*/

// int transmitter(char *interfaceName, char *macString, char *code, char *identifier);
// int listener(char *interfaceName, int myIdentifier);
// int hexToAscii(uint8_t *buf, int numbytes, int *combinedValue, char *asciiString);
// int twoBytesToInt(uint8_t *buf, int byte1, int byte2);
// void obtenerDireccionMAC(uint8_t *buf, char *macDestino);

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Error en argumentos.\n\n");
    printf("seth INTERFACE-SALIDA MAC-DESTINO (Formato XXXXXXXXXXXX) code (Formato xx) IDENTIFIER-DESTINO (Formato xx)\n\n");
    exit(1);
  }
  int myIdentifier = 88;
  char myIdentifierString[2];
  sprintf(myIdentifierString, "%d", myIdentifier);
  transmitter(argv[1], argv[2], argv[3], argv[4]);
  listener(argv[1], myIdentifier);

  return 0;
}
