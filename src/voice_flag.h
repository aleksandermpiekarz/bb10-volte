#ifndef VOICE_FLAG_H
#define VOICE_FLAG_H

#include <stddef.h>

/* Plan one PPS attribute update, preserving all other bits. The caller supplies
 * qualification (LTE/SIM/data/registered IMS), and performs the actual write.
 * Returns 1 for an update, 0 for no change, -1 for invalid/incomplete input.
 * No command is returned on failure or no change. */
int voice_flag_command(const char *snapshot, int qualified, char *out, size_t size);

/* Hold this descriptor until process exit; CLOEXEC keeps IMS children from
 * retaining the lock. Do not unlink an active lock file. */
int voice_flag_lock(const char *path);

#endif
