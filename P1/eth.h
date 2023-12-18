#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>

#define ETHER_TYPE 0x0070
#define BUF_SIZ 71024
#define MI_PROTOCOLO 47
#define MAC_STRING_SIZE 18

int twoBytesToInt(uint8_t *buf, int byte1, int byte2);
int listener(char *interfaceName, int myIdentifier);
void ConvierteMAC(char *Mac, char *Org);
void obtenerDireccionMAC(uint8_t *buf, char *macDestino);
int transmitter(char *interfaceName, char *macStringDestino, char *code, char *identifierDestino, int myIdentifier);
int listener(char *interfaceName, int myIdentifier);
int hexToAscii(uint8_t *buf, int numbytes, int *combinedValue, char *asciiString);

int twoBytesToInt(uint8_t *buf, int byte1, int byte2)
{
  return (buf[byte1] - '0') * 10 + (buf[byte2] - '0');
}

void ConvierteMAC(char *Mac, char *Org)
{ /*En lugar de caracteres, se requiere el numero*/
  int i, j, Aux, Acu;
  for (i = 0, j = 0, Acu = 0; i < 12; i++)
  {
    if ((Org[i] > 47) && (Org[i] < 58))
      Aux = Org[i] - 48;
    if ((Org[i] > 64) && (Org[i] < 97))
      Aux = Org[i] - 55;
    if (Org[i] > 96)
      Aux = Org[i] - 87;
    if ((i % 2) == 0)
      Acu = Aux * 16;
    else
    {
      Mac[j] = Acu + Aux;
      j++;
    }
  }
}

void obtenerDireccionMAC(uint8_t *buf, char *macDestino)
{
  buf += 20;

  // Convertimos los siguientes 12 bytes en formato ASCII a una cadena
  sprintf(macDestino, "%c%c%c%c%c%c%c%c%c%c%c%c",
          buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
          buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
}

int transmitter(char *interfaceName, char *macStringDestino, char *code, char *identifierDestino, int myIdentifier)
{
  int sockfd;
  struct ifreq if_idx;
  struct ifreq if_mac;
  int tx_len = 0, i, iLen;
  char sendbuf[BUF_SIZ], Mac[6];
  struct ether_header *eh = (struct ether_header *)sendbuf;
  struct sockaddr_ll socket_address;
  char ifName[IFNAMSIZ];
  char myIdentifierString[10];
  sprintf(myIdentifierString, "%d", myIdentifier);
  /*Coloca el nombre de la interface en ifName*/

  /*Abre el socket, notemos los parametros empleados*/
  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    perror("socket");

  /* Mediante el nombre de la interface, se obtiene su indice */
  memset(&if_idx, 0, sizeof(struct ifreq));              /*Llena de ceros el bloque de if_idx*/
  strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ - 1); /*Copia el nombre de la interfaz*/
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    perror("SIOCGIFINDEX"); /*Toma el control del driver*/

  /*Ahora obtenemos la MAC de la interface por donde saldran los datos */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
    perror("SIOCGIFHWADDR");
  /*Se imprime la MAC del host*/
  printf("Iterface de salida: %d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 0xFF & if_idx.ifr_ifindex,
         0xFF & if_mac.ifr_hwaddr.sa_data[0], 0xFF & if_mac.ifr_hwaddr.sa_data[1], 0xFF & if_mac.ifr_hwaddr.sa_data[2],
         0xFF & if_mac.ifr_hwaddr.sa_data[3], 0xFF & if_mac.ifr_hwaddr.sa_data[4], 0xFF & if_mac.ifr_hwaddr.sa_data[5]);

  /* Ahora se construye el encabezado Ethernet */
  memset(sendbuf, 0, BUF_SIZ); /*Llenamos con 0 el buffer de datos (payload)*/
  /*Direccion Origen*/
  eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
  eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
  eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
  eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
  eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
  eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
  /*Direccion destino*/

  ConvierteMAC(Mac, macStringDestino);

  eh->ether_dhost[0] = Mac[0];
  eh->ether_dhost[1] = Mac[1];
  eh->ether_dhost[2] = Mac[2];
  eh->ether_dhost[3] = Mac[3];
  eh->ether_dhost[4] = Mac[4];
  eh->ether_dhost[5] = Mac[5];

  char miMACString[MAC_STRING_SIZE];
  sprintf(miMACString, "%02x%02x%02x%02x%02x%02x",
          0xFF & if_mac.ifr_hwaddr.sa_data[0],
          0xFF & if_mac.ifr_hwaddr.sa_data[1],
          0xFF & if_mac.ifr_hwaddr.sa_data[2],
          0xFF & if_mac.ifr_hwaddr.sa_data[3],
          0xFF & if_mac.ifr_hwaddr.sa_data[4],
          0xFF & if_mac.ifr_hwaddr.sa_data[5]);

  if (strcmp(code, "10") == 0)
  {
    printf("Paquete ARP para saber MAC del nodo con identificador: %s\n", identifierDestino);
    char Cadena[] = "Quiero saber tu MAC";
    eh->ether_type = htons(strlen(code) + strlen(identifierDestino) + strlen(myIdentifierString) + strlen(miMACString) + strlen(Cadena));
    tx_len += sizeof(struct ether_header);
    strcpy(sendbuf + tx_len, code);
    tx_len = tx_len + strlen(code);
    strcpy(sendbuf + tx_len, identifierDestino);
    tx_len = tx_len + strlen(identifierDestino);
    strcpy(sendbuf + tx_len, myIdentifierString);
    tx_len = tx_len + strlen(myIdentifierString);
    strcpy(sendbuf + tx_len, miMACString);
    tx_len = tx_len + strlen(miMACString);
    strcpy(sendbuf + tx_len, Cadena);
    tx_len = tx_len + strlen(Cadena);
  }
  else if (strcmp(code, "20") == 0)
  {
    printf("Paquete ARP para para decirle al nodo con MAC:%s, mi MAC\n", macStringDestino);
    char Cadena[] = "Aquí va mi MAC";
    eh->ether_type = htons(strlen(code) + strlen(identifierDestino) + strlen(myIdentifierString) + strlen(miMACString) + strlen(Cadena));
    tx_len += sizeof(struct ether_header);
    strcpy(sendbuf + tx_len, code);
    tx_len = tx_len + strlen(code);
    strcpy(sendbuf + tx_len, identifierDestino);
    tx_len = tx_len + strlen(identifierDestino);
    strcpy(sendbuf + tx_len, myIdentifierString);
    tx_len = tx_len + strlen(myIdentifierString);
    strcpy(sendbuf + tx_len, miMACString);
    tx_len = tx_len + strlen(miMACString);
    strcpy(sendbuf + tx_len, Cadena);
    tx_len = tx_len + strlen(Cadena);
  }
  else
  {
    printf("Este es un mensaje cualquiera\n");
    char Cadena[] = "Este es un mensaje cualquiera";
    eh->ether_type = htons(strlen(code) + strlen(identifierDestino) + strlen(Cadena));
    tx_len += sizeof(struct ether_header);
    strcpy(sendbuf + tx_len, code);
    tx_len = tx_len + strlen(code);
    strcpy(sendbuf + tx_len, identifierDestino);
    tx_len = tx_len + strlen(identifierDestino);
    strcpy(sendbuf + tx_len, Cadena);
    tx_len = tx_len + strlen(Cadena);
  }

  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  socket_address.sll_halen = ETH_ALEN;
  socket_address.sll_addr[0] = Mac[0];
  socket_address.sll_addr[1] = Mac[1];
  socket_address.sll_addr[2] = Mac[2];
  socket_address.sll_addr[3] = Mac[3];
  socket_address.sll_addr[4] = Mac[4];
  socket_address.sll_addr[5] = Mac[5];

  /*Envio del paquete*/
  iLen = sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));
  if (iLen < 0)
    printf("Send failed\n");
  printf("Se ha enviado un paquete de %x bytes de payload...\n", eh->ether_type);
  for (i = 0; i < iLen; i++)
    printf("%02x ", sendbuf[i]);
  printf("\n\n\n");

  /*Cerramos*/
  close(sockfd);
  // * Si el código es 10 quiere decir que pidió MAC por lo que hay que escuchar
  if (strcmp(code, "10") == 0)
  {
    listener(interfaceName, myIdentifier);
  }
}

int listener(char *interfaceName, int myIdentifier)
{
  int sockfd, i;
  ssize_t numbytes;
  struct ifreq ifopts;
  struct ifreq if_ip;
  uint8_t buf[BUF_SIZ];
  char MiMAC[6];
  int saddr_size;
  struct sockaddr saddr;
  struct ifreq if_idx;
  struct ifreq if_mac;
  char data_received[256];
  int flag = 1;
  int code;

  int combinedValue;
  /*El encabezado del buffer en la estructura Ethernet*/
  struct ether_header *eh = (struct ether_header *)buf;

  memset(&if_ip, 0, sizeof(struct ifreq)); /*Llenamos de ceros*/

  /* Se abre el socket para "escuchar" los ETHER_TYPE */
  if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
  {
    perror("Listener: socket");
    return -1;
  }

  /* Mediante el nombre de la interface, se obtiene su indice */
  memset(&if_idx, 0, sizeof(struct ifreq));              /*Llena de ceros el bloque de if_idx*/
  strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ - 1); /*Copia el nombre de la interfaz*/
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    perror("SIOCGIFINDEX"); /*Toma el control del driver*/

  /*Ahora obtenemos la MAC de la interface por donde saldran los datos */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
    perror("SIOCGIFHWADDR");
  /*Se imprime la MAC del host*/
  for (i = 0; i < (IFNAMSIZ - 1); i++)
    MiMAC[i] = 0xFF & if_mac.ifr_hwaddr.sa_data[i];
  printf("Direccion MAC de la interfaz de entrada: %d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 0xFF & if_idx.ifr_ifindex,
         MiMAC[0], MiMAC[1], MiMAC[2], MiMAC[3], MiMAC[4], MiMAC[5]);

  do
  {
    saddr_size = sizeof saddr;
    /*Estamos escuchando por todas las interfaces del host*/
    numbytes = recvfrom(sockfd, buf, 65536, 0, &saddr, (socklen_t *)&saddr_size);
    int myIdentifierBUF = twoBytesToInt(buf, 16, 17);
    if ((numbytes == 51 && myIdentifier == myIdentifierBUF) || (numbytes == 47 && myIdentifier == myIdentifierBUF) || (numbytes == 48 && myIdentifier == myIdentifierBUF))
    {
      int code = twoBytesToInt(buf, 14, 15);
      if (code == 10)
      {
        char macDestinationFromBuf[MAC_STRING_SIZE];
        obtenerDireccionMAC(buf, macDestinationFromBuf);
        printf("QUIEREN SABER MI MAC\nENVIARE MI MAC A: %s\n\n", macDestinationFromBuf);
        int identifierDestination = twoBytesToInt(buf, 18, 19);
        char identifierDestinationString[2];
        sprintf(identifierDestinationString, "%d", identifierDestination);
        printf("ENNVIAR A: %s\n", identifierDestinationString);
        sleep(1);
        transmitter(interfaceName, macDestinationFromBuf, "20", identifierDestinationString, myIdentifier);
      }
      else if (code == 20)
      {
        printf("RECIBÍ MAC\n");
        char macDestinationFromBuf[MAC_STRING_SIZE];
        obtenerDireccionMAC(buf, macDestinationFromBuf);
        printf("YA SÉ TU MAC, ES: %s\n", macDestinationFromBuf);
        flag = 0;
      }
      else
      {
        printf("RECIBÍ MENSAJE CUALQUIERA\n");
        for (i = 0; i < numbytes; i++)
          printf("%02x ", buf[i]);
        printf("\n");
      }
    }

  } while (flag);

  close(sockfd);
}

int hexToAscii(uint8_t *buf, int numbytes, int *combinedValue, char *asciiString)
{
  // Verificamos que haya suficientes bytes en el búfer
  if (numbytes < 17)
  {
    printf("Error: Búfer demasiado corto.\n");
    return -1;
  }

  // Convertir los bytes 15 y 16 a un solo valor int
  *combinedValue = (buf[14] - '0') * 10 + (buf[15] - '0');

  // Construir el string ASCII con el resto de los bytes
  int i;
  for (i = 0; i < numbytes - 2; i++)
  {
    asciiString[i] = buf[i];
  }

  // Agregar el carácter nulo al final del string
  asciiString[i] = '\0';
  return 0;
}
