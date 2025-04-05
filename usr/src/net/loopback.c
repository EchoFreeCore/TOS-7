/*
 * loopback.c
 *
 * Drivrutin för loopback-interfacet (lo0). Denna drivrutin är
 * designad för att vara enkel och effektiv – i sann Unix V7-anda –
 * och anropar direkt net_stack_rx() för att leverera paketet
 * tillbaka till nätverksstacken.
 *
 * Inga onödiga kopieringar eller omvägar, bara ren, direkt
 * kommunikation för intern testning och debugging.
 */

 #include "iface.h"
 #include "net_stack.h"
 #include "kmalloc.h"
 #include <string.h>
 
 /*
  * loopback_send()
  *
  * Skickar data via loopback-interfacet. Eftersom det är
  * loopback kopieras datan minimalt (vi allokerar ett temporärt buffer)
  * och skickar sedan direkt till net_stack_rx().
  *
  * Parametrar:
  *   iface - Pekare till loopback-interfacet (bör heta "lo0").
  *   data  - Pekare till datan som ska skickas.
  *   len   - Längden på datan.
  *
  * Returnerar 0 vid lyckat skick, annars negativt fel.
  */
 int loopback_send(net_if_t *iface, const void *data, size_t len) {
     void *buffer = kmalloc(len);
     if (!buffer)
         return -1;
     
     memcpy(buffer, data, len);
     net_stack_rx(iface, buffer, len);
     kfree(buffer);
     return 0;
 }
 
 /*
  * loopback_init()
  *
  * Initierar loopback-drivrutinen genom att koppla om
  * send()-funktionen i det registrerade loopback-interfacet.
  * Detta görs under systemets initsekvens.
  */
 void loopback_init(void) {
     net_if_t *lo = if_lookup("lo0");
     if (lo) {
         lo->send = loopback_send;
     }
 }
 