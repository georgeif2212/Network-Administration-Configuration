#include "eth.h"

int main(int argc, char *argv[])
{
  int sockfd;
  struct ifreq if_idx;
  struct ifreq if_mac;
  int tx_len = 0, i, iLen;
  char sendbuf[BUF_SIZ], Mac[6];
  struct ether_header *eh = (struct ether_header *)sendbuf;
  struct sockaddr_ll socket_address;
  char ifName[IFNAMSIZ];
  char Cadena[]= "Hola mi paquete será de 300 bytes, esta es basura, un poco más.";

  if (argc != 3)
  {
    printf("Error en argumentos.\n\n");
    printf("seth INTERFACE-SALIDA MAC-DESTINO (Formato XXXXXXXXXXXX)\n\n");
    exit(1);
  }

  /*Coloca el nombre de la interface en ifName*/
  strcpy(ifName, argv[1]);

  /*Abre el socket, notemos los parametros empleados*/
  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    perror("socket");

  /* Mediante el nombre de la interface, se obtiene su indice */
  memset(&if_idx, 0, sizeof(struct ifreq));       /*Llena de ceros el bloque de if_idx*/
  strncpy(if_idx.ifr_name, ifName, IFNAMSIZ - 1); /*Copia el nombre de la interfaz*/
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    perror("SIOCGIFINDEX"); /*Toma el control del driver*/

  /*Ahora obtenemos la MAC de la interface por donde saldran los datos */
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, ifName, IFNAMSIZ - 1);
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
  ConvierteMAC(Mac, argv[2]);
  eh->ether_dhost[0] = Mac[0];
  eh->ether_dhost[1] = Mac[1];
  eh->ether_dhost[2] = Mac[2];
  eh->ether_dhost[3] = Mac[3];
  eh->ether_dhost[4] = Mac[4];
  eh->ether_dhost[5] = Mac[5];

  /* Rellenamos el paquete con basura*/
  eh->ether_type = htons(strlen(Cadena)); /*Recordemos, va al protocolo o la longitud del paquete*/
  tx_len += sizeof(struct ether_header);
  strcpy (sendbuf+tx_len, Cadena);
  tx_len = tx_len + strlen(Cadena);
  // strcpy(Cadena, "Hola mi paquete será de 300 bytes, esta es basura, un poco más.");
  // int cadena_len = strlen(Cadena);
  // if (cadena_len < 300)
  // {
  //   for (int i = cadena_len; i < 300; i++)
  //   {
  //     Cadena[i] = 'X';
  //   }
  // }
  // strcpy(sendbuf + tx_len, Cadena);
  // tx_len = tx_len + 300;

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
  printf("\n");

  /*Cerramos*/
  close(sockfd);
  return 0;
}