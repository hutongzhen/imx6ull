#define _POSIX_C_SOURCE 200809L

#include "GetTime.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <poll.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static int monotonic_ms(int64_t *value)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        return -1;
    *value = (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return 0;
}

/* poll sleeps while waiting. All phases share one monotonic deadline, so
 * slow peers and repeated interrupts cannot extend the request indefinitely.
 */
static int wait_ready(int fd, short events, int64_t deadline)
{
    for (;;) {
        int64_t now;
        if (monotonic_ms(&now) < 0)
            return -1;
        if (now >= deadline) {
            errno = ETIMEDOUT;
            return -1;
        }
        int64_t remaining = deadline - now;
        struct pollfd pfd = { .fd = fd, .events = events };
        int result = poll(&pfd, 1,
                          remaining > INT_MAX ? INT_MAX : (int)remaining);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (result == 0)
            continue;
        if (pfd.revents & POLLNVAL) {
            errno = EBADF;
            return -1;
        }
        /* Let connect/recv/send report the actual socket error or EOF. */
        if (pfd.revents & (events | POLLERR | POLLHUP))
            return 0;
    }
}

static int parse_time(char *reply, size_t length, struct timespec *result)
{
    /* Exactly: OK <nonnegative decimal seconds>\n, with optional CRLF. */
    if (length && reply[length - 1] == '\r')
        --length;
    if (length <= 3 || memcmp(reply, "OK ", 3) != 0) {
        errno = EPROTO;
        return -1;
    }
    for (size_t i = 3; i < length; ++i) {
        if (reply[i] < '0' || reply[i] > '9') {
            errno = EPROTO;
            return -1;
        }
    }
    reply[length] = '\0';
    errno = 0;
    intmax_t seconds = strtoimax(reply + 3, NULL, 10);
    if (errno == ERANGE)
        return -1;

    /* This board's old libc commonly has a signed 32-bit time_t. Reject
     * unrepresentable dates rather than silently wrapping past January 2038.
     */
    time_t converted = (time_t)seconds;
    if (converted < (time_t)0 || (intmax_t)converted != seconds) {
        errno = EOVERFLOW;
        return -1;
    }
    result->tv_sec = converted;
    result->tv_nsec = 0;
    return 0;
}

int GetTime(const char *server_ip, uint16_t port, int timeout_ms)
{
    static const char request[] = "GET_TIME\n";
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (!server_ip || !port || timeout_ms <= 0 ||
        inet_pton(AF_INET, server_ip, &address.sin_addr) != 1) {
        errno = EINVAL;
        return -1;
    }

    int64_t started;
    if (monotonic_ms(&started) < 0)
        return -1;
    int64_t deadline = started + timeout_ms;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    int result = -1;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        goto done;

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        if (errno != EINPROGRESS && errno != EINTR)
            goto done;
        if (wait_ready(fd, POLLOUT, deadline) < 0)
            goto done;
        int socket_error = 0;
        socklen_t size = sizeof(socket_error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &socket_error, &size) < 0)
            goto done;
        if (socket_error) {
            errno = socket_error;
            goto done;
        }
    }

    size_t sent = 0;
    while (sent < sizeof(request) - 1) {
        if (wait_ready(fd, POLLOUT, deadline) < 0)
            goto done;
        ssize_t n = send(fd, request + sent, sizeof(request) - 1 - sent,
                         MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                continue;
            goto done;
        }
        if (n == 0) {
            errno = EPIPE;
            goto done;
        }
        sent += (size_t)n;
    }

    char reply[96];
    size_t used = 0;
    for (;;) {
        if (wait_ready(fd, POLLIN, deadline) < 0)
            goto done;
        ssize_t n = recv(fd, reply + used, sizeof(reply) - 1 - used, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                continue;
            goto done;
        }
        if (n == 0) {
            errno = EPROTO; /* EOF before a complete newline-terminated reply. */
            goto done;
        }
        used += (size_t)n;
        char *newline = memchr(reply, '\n', used);
        if (newline) {
            size_t length = (size_t)(newline - reply);
            struct timespec server_time;
            if (length + 1 != used) {
                errno = EPROTO;
                goto done;
            }
            if (parse_time(reply, length, &server_time) < 0)
                goto done;
            /* Only this final step changes the clock. No timezone offset:
             * Unix seconds represent the same instant in every timezone.
             */
            result = clock_settime(CLOCK_REALTIME, &server_time);
            goto done;
        }
        if (used == sizeof(reply) - 1) {
            errno = EMSGSIZE;
            goto done;
        }
    }

done:
    {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
    }
    return result;
}
