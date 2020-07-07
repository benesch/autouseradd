#!/usr/bin/env bats

container() {
  docker run --entrypoint=autouseradd --rm "$@"
}

@test "invalid arguments" {
  run container autouseradd --foo
  [[ "$status" -eq 1 ]]
  [[ "$output" = "autouseradd: unrecognized option '--foo'" ]]
}

@test "version" {
  container autouseradd -v        | grep -E "^autouseradd [0-9]+\.[0-9]+\.[0-9]+$"
  container autouseradd --version | grep -E "^autouseradd [0-9]+\.[0-9]+\.[0-9]+$"
}

@test "prevent root" {
  run container autouseradd newuser
  [[ "$status" -eq 1 ]]
  [[ "$output" = "autouseradd: running as root is not permitted" ]]
}

@test "new uid/gid" {
  run container -u 501:502 autouseradd id
  [[ "$status" -eq 0 ]]
  [[ "$output" = "uid=501(auto) gid=502(auto) groups=502(auto)" ]]
}

@test "new uid/gid with custom names" {
  run container -u 504:503 autouseradd -u a -g b id
  [[ "$status" -eq 0 ]]
  [[ "$output" = "uid=504(a) gid=503(b) groups=503(b)" ]]
}

@test "default shell" {
  run container -i -u 1001:1001 autouseradd <<< 'echo $BASH'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "/bin/bash" ]]
}

@test "custom shell" {
  run container -i -u 1000:1001 autouseradd -s /bin/sh <<< 'echo $0'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "/bin/sh" ]]
}

@test "chdir" {
  run container -u 501:502 autouseradd -C /home/auto pwd
  [[ "$status" -eq 0 ]]
  [[ "$output" = "/home/auto" ]]
}

@test "--no-create-home" {
  run container -u 501:502 autouseradd --no-create-home ls -l /home
  [[ "$status" -eq 0 ]]
  [[ "$output" = "total 0" ]]
}

@test "home directory permissions" {
  run container -u 501:502 autouseradd sh -c 'stat -c "%A %n" $HOME'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "drwxr-xr-x /home/auto" ]]
}

@test "home directory exists" {
  run container --volume=foo:/home/auto -u 501:502 autouseradd true
  [[ "$status" -eq 0 ]]
  [[ "$output" = *"warning: the home directory already exists"* ]]
}

@test "home directory exists with --no-create-home" {
  run container --volume=foo:/home/auto -u 501:502 autouseradd --no-create-home true
  [[ "$status" -eq 0 ]]
  [[ -z "$output" ]]
}

@test "env propagates" {
  run container -u 501:502 -e FOO=bar autouseradd sh -c 'echo $FOO'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "bar" ]]
}

@test "env overrides" {
  run container -u 501:502 -e FOO=outside autouseradd -e FOO=inside1 -e FOO=inside2 sh -c 'echo $FOO'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "inside2" ]]
}

@test "env unset" {
  run container -u 501:502 -e FOO=outside autouseradd -e FOO sh -uc 'echo $FOO'
  [[ "$status" -eq 2 ]]
  [[ "$output" = "sh: 1: FOO: parameter not set" ]]
}

@test "tmpdir propagates" {
  run container -u 501:502 -e TMPDIR=/foo autouseradd sh -c 'echo $TMPDIR'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "/foo" ]]
}

@test "tmpdir overrides" {
  run container -u 501:502 -e TMPDIR=/outside autouseradd -e TMPDIR=/inside sh -c 'echo $TMPDIR'
  [[ "$status" -eq 0 ]]
  [[ "$output" = "/inside" ]]
}

@test "duplicate gid" {
  run container -u 501:1 autouseradd
  [[ "$status" -eq 1 ]]
  [[ "${lines[0]}" = "groupadd: GID '1' already exists" ]]
}

@test "duplicate uid" {
  run container -u 1:501 autouseradd
  [[ "$status" -eq 1 ]]
  [[ "${lines[0]}" = "useradd: UID 1 is not unique" ]]
}
