#include "beep.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BEEP_PATH "/sys/class/leds/beep/"

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

static int beep_set(const char *brightness)
{
    if (write_attribute(BEEP_PATH "trigger", "none\n") < 0)
        return -1;

    return write_attribute(BEEP_PATH "brightness", brightness);
}

int beep_on(void)
{
    return beep_set("255\n");
}

int beep_off(void)
{
    return beep_set("0\n");
}
