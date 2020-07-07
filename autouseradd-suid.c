#define _GNU_SOURCE

#include <err.h>
#include <getopt.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char* version = "1.1.0";

const uid_t root_uid = 0;
const gid_t root_gid = 0;

const struct option long_options[] = {
  {"user",           required_argument, 0, 'u'},
  {"group",          required_argument, 0, 'g'},
  {"shell",          required_argument, 0, 's'},
  {"cwd",            required_argument, 0, 'C'},
  {"env",            required_argument, 0, 'e'},
  {"no-create-home", no_argument,       0, 'M'},
  {"help",           no_argument,       0, 'h'},
  {"version",        no_argument,       0, 'v'},
  {0, 0, 0, 0}
};

void run(char* args[]) {
  pid_t r;
  if ((r = fork()) == 0) {
    execv(args[0], args);
    err(1, "exec");
  } else if (r == -1)
    err(1, "fork");
  int wstatus;
  if (waitpid(r, &wstatus, 0) == -1)
    err(1, "waitpid");
  if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != EXIT_SUCCESS)
    errx(1, "%s failed", args[0]);
}

char* itoa(int id) {
  char* s;
  if (asprintf(&s, "%d", id) == -1)
    err(1, "asprintf");
  return s;
}

void usage(void) {
  printf("synopsis: autouseradd [-u NAME] [-g NAME] [COMMAND [ARGS]]\n"
    "\n"
    "autouseradd adds a user and group named after that user to the system,\n"
    "then invokes a command as that user.\n"
    "\n"
    "Options:\n"
    "  -u, --user NAME       use NAME as the username for the created user\n"
    "  -g, --group NAME      use NAME as the group name for the created user\n"
    "  -s, --shell PATH      use PATH as the user's login shell\n"
    "  -c, --cwd PATH        set the working directory to PATH\n"
    "  -e, --env NAME=VALUE  set the environment variable NAME to VALUE\n"
    "  --no-create-home      do not attempt to create a home directory for the user\n"
    "  --help                print this help and exit\n"
    "  --version             print the version and exit\n"
    "\n"
    "See the man page for a more complete description.\n");
}

int main(int argc, char* argv[]) {
  char* user = "auto";
  char* group = NULL;
  char* shell = "/bin/bash";
  int envc = 0;
  char* envs[argc];
  char* cwd = NULL;
  bool create_home = true;

  int c;
  while ((c = getopt_long(argc, argv, "+u:g:s:C:e:hv", long_options, NULL)) != -1) {
    switch (c) {
      case 'u':
        user = optarg;
        break;
      case 'g':
        group = optarg;
        break;
      case 's':
        shell = optarg;
        break;
      case 'C':
        cwd = optarg;
        break;
      case 'e':
        envs[envc++] = optarg;
        break;
      case 'M':
        create_home = false;
        break;
      case 'h':
        usage();
        exit(0);
      case 'v':
        printf("autouseradd %s\n", version);
        exit(0);
      case '?':
        exit(1);
      default:
        errx(1, "option parsing failed");
      }
  }
  if (group == NULL)
    group = user;

  uid_t ruid, euid, suid, rgid, egid, sgid;
  if (getresuid(&ruid, &euid, &suid) != 0)
    err(1, "getresuid");
  if (getresgid(&rgid, &egid, &sgid) != 0)
    err(1, "getresgid");
  if (ruid == root_uid || rgid == root_gid)
    errx(1, "running as root is not permitted");
  if (euid != root_uid)
    errx(1, "setuid bit not set: %d", euid);

  char* ruids = itoa(ruid);
  char* rgids = itoa(rgid);
  run((char* []){"/usr/sbin/groupadd", "--gid", rgids, group, NULL});
  run((char* []){"/usr/sbin/useradd", "--no-user-group",
    create_home ? "--create-home" : "--no-create-home",
    "--uid", ruids, "--gid", rgids, "--shell", shell, user, NULL});
  free(ruids);
  free(rgids);

  struct passwd* pwent = getpwuid(ruid);
  if (pwent == NULL)
    err(1, "getpwuid");
  if (pwent->pw_dir == NULL || strlen(pwent->pw_dir) == 0)
    errx(1, "user created without home directory");
  setenv("HOME", pwent->pw_dir, 1 /* overwrite */);

  if (setresuid(ruid, ruid, ruid) == -1)
    err(1, "setresuid");
  if (setresgid(rgid, rgid, rgid) == -1)
    err(1, "setresgid");

  if (cwd != NULL && chdir(cwd) == -1)
    err(1, "chdir");

  for (int i = 0; i < envc; i++)
    putenv(envs[i]);

  if (argc == optind)
    execl(shell, shell, (char*) NULL);
  else
    execvp(argv[optind], &argv[optind]);
  err(1, "exec");
}
