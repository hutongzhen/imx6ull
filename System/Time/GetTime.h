#ifndef SYSTEM_TIME_GET_TIME_H
#define SYSTEM_TIME_GET_TIME_H

#include <stdint.h>

/* Request UTC Unix seconds from the TCP time server and update CLOCK_REALTIME.
 * server_ip: numeric IPv4 address; port: TCP port in host byte order.
 * timeout_ms: total connection/send/receive deadline in milliseconds (> 0).
 * Returns 0 on success, -1 with errno set on failure. Requires root or
 * CAP_SYS_TIME to set the clock. Invalid/network replies do not change it.
 * Sets only the system clock, not the hardware RTC or the time zone.
 * Intended for the trusted lab network; this protocol is not authenticated.
 */
int GetTime(const char *server_ip, uint16_t port, int timeout_ms);

#endif
