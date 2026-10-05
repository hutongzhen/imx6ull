#include "led.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define LED_PATH "/sys/class/leds/sys-led/"

static int write_attribute(const char *path, const char *value)
{
    int fd;
    int saved_errno;
    size_t length = strlen(value);
    ssize_t written;

    fd = open(path, O_WRONLY);
    if (fd < 0)
        return -1;

    do {
        written = write(fd, value, length);
    } while (written < 0 && errno == EINTR);

    if (written != (ssize_t)length) {
        saved_errno = written < 0 ? errno : EIO;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    return close(fd);
}

static int led_set(const char *brightness)
{
    return write_attribute(LED_PATH "brightness", brightness);
}

int led_on(void)
{
    return led_set("255\n");
}

int led_off(void)
{
    return led_set("0\n");
}
