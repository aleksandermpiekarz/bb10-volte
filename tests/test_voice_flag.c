#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "voice_flag.h"

static int checks, failures;
#define CHECK(c) do { ++checks; if (!(c)) { ++failures; fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); } } while (0)

/* Missing qualification must clear only the IMS voice bit. Expectations are
 * taken from observed device masks, not computed by the implementation. */
int main(void)
{
    char out[128];
    CHECK(voice_flag_command("@status_public\nnetwork_service:n:2098186\n", 1, out, sizeof(out)) == 1);
    CHECK(strcmp(out, "network_service:n:35652618\n") == 0);
    CHECK(voice_flag_command("@status_public\n[n]network_service:n:35652618\n", 0, out, sizeof(out)) == 1);
    CHECK(strcmp(out, "network_service:n:2098186\n") == 0);
    CHECK(voice_flag_command("network_service:n:35652618\n", 1, out, sizeof(out)) == 0);
    CHECK(voice_flag_command("network_service:n:2098186\n", 0, out, sizeof(out)) == 0);
    CHECK(voice_flag_command("network_service:n:8194\n", 1, out, sizeof(out)) == 0);
    CHECK(voice_flag_command("network_service:n:33562626\n", 1, out, sizeof(out)) == 1);
    CHECK(strcmp(out, "network_service:n:8194\n") == 0);
    CHECK(voice_flag_command("[n]@status_public\n[n]network_service:n:2098186\r\nother::data\n", 1, out, sizeof(out)) == 1);
    CHECK(strcmp(out, "network_service:n:35652618\n") == 0);
    CHECK(voice_flag_command("network_service:n:4294967295\n", 0, out, sizeof(out)) == 1);
    CHECK(strcmp(out, "network_service:n:4261412863\n") == 0);
    const char *invalid[] = {
        "", "network_service:n:\n", "network_service:n:-1\n",
        "network_service:n:+2098186\n", "network_service:n:4294967296\n",
        "network_service:n:99999999999999999999999999\n",
        "network_service:n:2098186junk\n", "network_service::2098186\n",
        "network_service:n:2098186",
        "other_network_service:n:2098186\n", "x::network_service:n:2098186\n",
        "network_service:n:2098186\nnetwork_service:n:8194\n"
    };
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) {
        strcpy(out, "stale");
        CHECK(voice_flag_command(invalid[i], 1, out, sizeof(out)) == -1);
        CHECK(out[0] == '\0');
    }
    CHECK(voice_flag_command("network_service:n:2098186\n", 1, out, 4) == -1);
    CHECK(out[0] == '\0');
    CHECK(voice_flag_command(NULL, 1, out, sizeof(out)) == -1);
    CHECK(voice_flag_command("network_service:n:2098186\n", 1, NULL, 0) == -1);

    /* A second daemon must not touch PPS before noticing an existing owner. */
    char path[] = "/tmp/bb10-voice-lock-XXXXXX";
    int temp = mkstemp(path);
    CHECK(temp >= 0);
    if (temp >= 0) close(temp);
    int owner = voice_flag_lock(path);
    CHECK(owner >= 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (child == 0) {
        int second = voice_flag_lock(path);
        _exit(second < 0 ? 0 : 1);
    }
    int status = 0;
    CHECK(waitpid(child, &status, 0) == child);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    if (owner >= 0) close(owner);
    child = fork();
    CHECK(child >= 0);
    if (child == 0) {
        int replacement = voice_flag_lock(path);
        _exit(replacement >= 0 ? 0 : 1);
    }
    CHECK(waitpid(child, &status, 0) == child);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    unlink(path);
    printf("Voice flag: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
