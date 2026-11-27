/* Stubs for the desktop traffic test (test/traffic_test.c): car.c's three netdiag calls
 * (NETLOG and the versus flash) without net.c's sockets. */
#include "netdiag.h"
#include "car.h"

int net_log_on(void) { return 0; }
void net_set_flash(int off) { (void)off; }
int net_flash_of(int pcb) { (void)pcb; return 0; }
