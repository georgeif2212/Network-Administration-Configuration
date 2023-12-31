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
  if (argc != 6)
  {
    printf("Error en argumentos.\n\n");
    printf("seth INTERFACE-SALIDA MAC-DESTINO (Formato XXXXXXXXXXXX) code (Formato xx) IDENTIFIER-DESTINO MY-IDENTIFIER(Formato xx)\n\n");
    exit(1);
  }
  int myIdentifier = atoi(argv[5]);
  transmitter(argv[1], argv[2], argv[3], argv[4], myIdentifier);

  return 0;
}
