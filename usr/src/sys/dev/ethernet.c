/*
 * ethernet.c
 *
 * Drivrutin för Ethernet-interface (eth0). Denna drivrutin använder
 * DMA för att möjliggöra zero-copy sändning, och integrerar med den
 * arkitekturbundna drivrutinen (arch_eth_* funktionerna) för hårdvaruåtkomst.
 * Ethernet-drivrutinen initierar sitt interface, hanterar sändning via
 * ethernet_send() och tar emot paket via ethernet_receive(), som vidarebefordras
 * till nätverksstacken.
 */

 #include "iface.h"
 #include "net_stack.h"
 #include "kmalloc.h"
 #include "dma.h"      // Hypotetisk header för arkitekturbunden DMA-hantering
 #include <string.h>
 #include <stdint.h>
 
 #define ETH_IFACE_NAME "eth0"
 #define MAX_ETH_FRAME_SIZE 1536  // Maximal ramstorlek, anpassa vid behov
 
 /*
  * ethernet_send()
  *
  * Skickar en Ethernet-ram via DMA. Funktionen kontrollerar att ramstorleken
  * är inom tillåtet intervall och anropar den arkitekturbundna sändfunktionen.
  *
  * Parametrar:
  *   iface - Pekare till Ethernet-interfacet (eth0).
  *   data  - Pekare till Ethernet-rammen (inklusive Ethernet-huvud).
  *   len   - Längd på rammen i bytes.
  *
  * Returnerar 0 vid lyckad sändning, annars ett negativt fel.
  */
 int ethernet_send(net_if_t *iface, const void *data, size_t len) {
     if (len > MAX_ETH_FRAME_SIZE)
         return -1;
     
     // Anropa den arkitekturbundna DMA-sändningsfunktionen.
     if (arch_eth_send(data, len) < 0)
         return -1;
     
     return 0;
 }
 
 /*
  * ethernet_receive()
  *
  * Denna funktion anropas från en interrupt- eller polling-miljö när
  * en Ethernet-ram tas emot. Den mottagna rammen (i en zero-copy buffer)
  * skickas vidare direkt till nätverksstackens mottagningsfunktion.
  *
  * Parametrar:
  *   data - Pekare till mottagen Ethernet-ram.
  *   len  - Längden på rammen i bytes.
  */
 void ethernet_receive(const void *data, size_t len) {
     // Här kan man även extrahera och validera Ethernet-huvudet
     // (MAC-adresser, EtherType, etc.) innan vidarebefordran.
     net_if_t *eth_iface = if_lookup(ETH_IFACE_NAME);
     if (eth_iface)
         net_stack_rx(eth_iface, data, len);
 }
 
 /*
  * ethernet_init()
  *
  * Initierar Ethernet-drivrutinen:
  *   - Kallar på den arkitekturbundna initieringsrutinen för Ethernet/DMA.
  *   - Konfigurerar och registrerar Ethernet-interfacet ("eth0") i systemet.
  */
 void ethernet_init(void) {
     // Initiera den arkitekturbundna Ethernet/DMA-hårdvaran
     arch_eth_init();
     
     // Skapa och konfigurera Ethernet-interfacet
     static net_if_t eth0;
     memset(&eth0, 0, sizeof(eth0));
     
     strncpy(eth0.name, ETH_IFACE_NAME, IF_NAME_MAX);
     // IP-adress och netmask sätts senare (via DHCP eller statisk konfiguration)
     eth0.ip_addr = 0;
     eth0.netmask = 0;
     
     // Koppla sändfunktionen till ethernet_send()
     eth0.send = ethernet_send;
     
     // Registrera Ethernet-interfacet i systemet
     if_register(&eth0);
 }
 