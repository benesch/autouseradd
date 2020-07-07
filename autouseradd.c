#define _GNU_SOURCE

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define XSTR(s) #s
#define STR(s) XSTR(s)

// See the "Secure-execution-mode" subsection of the "ENVIRONMENT" section:
// https://man7.org/linux/man-pages/man8/ld.so.8.html#ENVIRONMENT
const char* envs[] = {
  "GCONV_PATH",
  "GETCONF_DIR",
  "HOSTALIASES",
  "LD_AUDIT",
  "LD_DEBUG",
  "LD_DYNAMIC_WEAK",
  "LD_LIBRARY_PATH",
  "LD_ORIGIN_PATH",
  "LD_PREFER_MAP_32BIT_EXEC",
  "LD_PROFILE_OUTPUT",
  "LD_PROFILE",
  "LD_USE_LOAD_BIAS",
  "LOCALDOMAIN",
  "LOCPATH",
  "MALLOC_TRACE",
  "NIS_PATH",
  "NLSPATH",
  "RES_OPTIONS",
  "RESOLV_HOST_CONF",
  "TMPDIR",
  "TZDIR"
};

const ssize_t envc = sizeof(envs) / sizeof(char*);

int main(int argc, char* argv[]) {
  int a = 0;
  char* args[argc + envc + 1];

  args[a++] = argv[0];

  for (int i = 0; i < envc; i++) {
    char* env = getenv(envs[i]);
    if (env != NULL)
      if (asprintf(&args[a++], "--env=%s=%s", envs[i], env) == -1)
        err(1, "asprintf");
  }

  for (int i = 1; i < argc; i++)
    args[a++] = argv[i];

  args[a++] = NULL;

  execv(STR(PREFIX) "/libexec/autouseradd-suid", args);
  err(1, "exec");
}
