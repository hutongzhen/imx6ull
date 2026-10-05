#ifndef HARDWARE_BEEP_H
#define HARDWARE_BEEP_H

/* Return 0 on success, or -1 with errno set. Disable automatic triggering. */
int beep_on(void);
int beep_off(void);

#endif
