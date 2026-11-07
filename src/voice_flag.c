#include "voice_flag.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define DATA_LTE UINT32_C(0x00200000)
#define IMS_VOICE UINT32_C(0x02000000)

int voice_flag_lock(const char *path)
{
    int fd = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0600);
    if (fd < 0)
        return -1;
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &lock) < 0) {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }
    return fd;
}

int voice_flag_command(const char *snapshot, int qualified, char *out, size_t size)
{
    const char key[] = "network_service:n:";
    uint32_t current = 0;
    int found = 0;
    if (!out || !size)
        return -1;
    out[0] = '\0';
    if (!snapshot)
        return -1;

    for (const char *p = snapshot; *p; ) {
        const char *end = strchr(p, '\n');
        if (!end) /* A truncated numeric attribute must never become a mask. */
            return -1;
        if (strncmp(p, "[n]", 3) == 0)
            p += 3;
        if (strncmp(p, key, sizeof(key) - 1) == 0) {
            if (found++)
                return -1;
            p += sizeof(key) - 1;
            if (p == end || *p < '0' || *p > '9')
                return -1;
            while (p < end && *p >= '0' && *p <= '9') {
                unsigned digit = (unsigned)(*p++ - '0');
                if (current > (UINT32_MAX - digit) / 10)
                    return -1;
                current = current * 10 + digit;
            }
            if (p < end && *p == '\r')
                ++p;
            if (p != end)
                return -1;
        }
        p = end + 1;
    }
    if (!found)
        return -1;
    uint32_t next = qualified && (current & DATA_LTE)
                        ? current | IMS_VOICE : current & ~IMS_VOICE;
    if (next == current)
        return 0;
    int n = snprintf(out, size, "network_service:n:%lu\n", (unsigned long)next);
    if (n < 0 || (size_t)n >= size) {
        out[0] = '\0';
        return -1;
    }
    return 1;
}
