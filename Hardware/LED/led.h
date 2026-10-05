#ifndef HARDWARE_LED_H
#define HARDWARE_LED_H

/* Turn the system LED on/off. Return 0 on success, or -1 with errno set. */
/* The caller must disable the automatic LED trigger before manual control. */
int led_on(void);
int led_off(void);

#endif
