/*Escucha todos los paquetes ethernet que llegan*/
#include "eth.h"

 int main(int argc, char *argv[])
 {
   int sockfd, i;
   ssize_t numbytes;
   struct ifreq ifopts; 
   struct ifreq if_ip; 
   uint8_t buf[BUF_SIZ];
   char ifName[IFNAMSIZ], MiMAC[6];
   int saddr_size; 
   struct sockaddr saddr;    
   struct ifreq if_idx;
   struct ifreq if_mac;
   int iEtherType;
  
   if (argc!=2)
   {
     printf ("Error en argumentos.\n\n");
     printf ("reth INTERFACE-ENTRADA\n\n");
     exit (1);
   }
  
   /*Coloca la interface*/
   strcpy(ifName, argv[1]);   

   /*El encabezado del buffer en la estructura Ethernet*/
   struct ether_header *eh = (struct ether_header *)buf;

   memset(&if_ip, 0, sizeof(struct ifreq));  /*Llenamos de ceros*/

   /* Se abre el socket para "escuchar" los ETHER_TYPE */
   if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) 
   {
     perror("Listener: socket"); 
     return -1;
   }

   /* Mediante el nombre de la interface, se obtiene su indice */
   memset (&if_idx, 0, sizeof(struct ifreq));  /*Llena de ceros el bloque de if_idx*/
   strncpy (if_idx.ifr_name, ifName, IFNAMSIZ-1);  /*Copia el nombre de la interfaz*/
   if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) perror("SIOCGIFINDEX"); /*Toma el control del driver*/
   
   /*Ahora obtenemos la MAC de la interface por donde saldran los datos */
   memset(&if_mac, 0, sizeof(struct ifreq));
   strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
   if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) perror("SIOCGIFHWADDR");
   /*Se imprime la MAC del host*/
   for (i=0; i<(IFNAMSIZ-1); i++) MiMAC[i] = 0xFF&if_mac.ifr_hwaddr.sa_data[i]; 
   printf ("Direccion MAC de la interfaz de entrada: %d, MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 0xFF&if_idx.ifr_ifindex, 
           MiMAC[0], MiMAC[1], MiMAC[2], MiMAC[3], MiMAC[4], MiMAC[5]);

   
   do {   
     saddr_size = sizeof saddr;
     /*Estamos escuchando por todas las interfaces del host*/
     numbytes = recvfrom(sockfd, buf, 65536, 0, &saddr, (socklen_t*)&saddr_size);  
     //if (numbytes==119) /*Payload + encabezado*/
     {
       printf ("Llego paquete de %d bytes: \n", numbytes);
       printf ("Host Destino: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                eh->ether_dhost[0], eh->ether_dhost[1], eh->ether_dhost[2],
                eh->ether_dhost[3], eh->ether_dhost[4], eh->ether_dhost[5]);
       printf ("Host Fuente: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                eh->ether_shost[0], eh->ether_shost[1], eh->ether_shost[2],
                eh->ether_shost[3], eh->ether_shost[4], eh->ether_shost[5]);    
       printf ("Ether Type: %04X\n2", htons(eh->ether_type));            
       for (i=0; i<numbytes; i++) printf("%02x ", buf[i]);
       printf("\n");                
     }
   } while (1);

   close(sockfd); 
   return (0);
 }