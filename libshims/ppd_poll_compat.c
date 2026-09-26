#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stddef.h>

/* The legacy daemon leaves its first pollfd uninitialized when the histogram
 * descriptor is unavailable. An invalid positive fd makes poll return
 * POLLNVAL immediately and spins one CPU core. */
int poll(struct pollfd *fds, nfds_t count, int timeout)
{
    static int (*real_poll)(struct pollfd *, nfds_t, int);
    int (*next_poll)(struct pollfd *, nfds_t, int);

    if (fds != NULL && count == 3 && fds[0].events == 0 && fds[0].fd >= 0) {
        int saved_errno = errno;
        if (fcntl(fds[0].fd, F_GETFD) < 0 && errno == EBADF)
            fds[0].fd = -1;
        errno = saved_errno;
    }

    next_poll = __atomic_load_n(&real_poll, __ATOMIC_ACQUIRE);
    if (next_poll == NULL) {
        next_poll = dlsym(RTLD_NEXT, "poll");
        __atomic_store_n(&real_poll, next_poll, __ATOMIC_RELEASE);
    }
    if (next_poll == NULL) {
        errno = ENOSYS;
        return -1;
    }
    return next_poll(fds, count, timeout);
}
