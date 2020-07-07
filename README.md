# autouseradd 👩‍🚒

autouseradd puts out fires started by `docker run --user UID [CMD...]`. 🔥

When used as a container's entrypoint, autouseradd ensures that `UID` has a
proper entry in `/etc/passwd` and a writable home directory before invoking
`CMD`.

By default, Docker makes no guarantee that `UID` specifies a valid user; rather,
it naïvely invokes `CMD` as the specified user ID. That UID won't have a
corresponding username in the system password database, `/etc/passwd`, nor will
it have a writable home directory.† Yet any sufficiently complicated application
will eventually include code that attempts to look up the current user's name or
write to `$HOME`, either failing your test suite or flat-out crashing.

<small>†To be pedantic, it's possible that `UID` has an `/etc/passwd` entry if
it happens to match the ID of a built-in user. Most Linux distributions ship
with several dozen.</small>

## Usage

If you base your images on a recent Linux distribution, you can download the
precompiled binaries for AMD64:

```dockerfile
# Dockerfile

FROM distro-foo

RUN curl -fsSL https://github.com/benesch/autouseradd/releases/download/1.2.0/autouseradd-1.2.0-amd64.tar.gz \
    | tar xz -C /usr/local --strip-components 1

ENTRYPOINT ["autouseradd", "--user", "fido"]
```

```bash
$ docker run -it --user 501:501 some-image id
uid=501(fido) gid=501(fido) groups=501(fido)
```

**Important:** Be sure to extract the tarball as root or `autouseradd`
won't have the necessary permissions to add users and groups to the system.

**Important:** Be sure to extract the tarball into `/usr/local`, as this path
is hardcoded into the precompiled binaries. To install into a different
prefix, compile from source instead and set the `PREFIX` Make variable
appropriately.

On older Linux distributions or non-AMD64 platforms, you'll need to compile
`autouseradd` from source:

```bash
$ curl -fsSL https://github.com/benesch/autouseradd/archive/1.2.0.tar.gz | tar xz
$ cd autouseradd-1.2.0 && make && sudo make install
```

You can wrap an existing entrypoint with `autouseradd`. If you were using
`jekyll` as your previous entrypoint, for example, simply specify it as the next
argument to `autouseradd`.

```dockerfile
ENTRYPOINT ["autouseradd", "--user", "jekyll", "jekyll"]
```

See [autouseradd.1](autouseradd.1) or `man autouseradd` for a full description
of the command.

## Motivation

When using Docker containers for development, binding your source code into your
container can be very convenient:

```bash
docker run -v "$PWD/src:/src" company/dev-image make build
```

Unfortunately, Docker runs as `root` by default, so your build process will
litter root-owned files and directories in your source tree. These can be a real
pain; you'll need `sudo` on the host to clean up your source tree.

You might try adding a non-root user to `company/dev-image`, but that doesn't
solve the problem. That non-root user almost certainly has a different UID than
your user account on your host machine, so you *still* won't be able to remove
Docker-created files from your source tree.

Instead, you have to dynamically choose a UID that matches your host UID when
booting the container:

```bash
docker run -v "$PWD/src:/src" -u "$(id -u):$(id -g)" company/dev-image ...
```

This works—files and directories are created with the right owner on the
host—until something in your build process fails when it can't find an entry for
the UID in `/etc/passwd`. More programs choke on missing password database
entries than you'd think.

For a concrete example, look at the [long list of `--user`-related caveats
associated with the library/postgres image][postgres-user-notes]. In short,
Postgres's `initdb` command requires a valid `/etc/passwd` file. The two
recommended solutions are to a) bind-mount your host's `/etc/passwd` into the
container, or b) run a container to run `initdb` as root, run a container to
chown the resulting database files to the desired user, then run a final
container as the desired user.

Option (b) is a kludge, and only works because the actual Postgres daemon
doesn't care whether `/etc/passwd` is valid.

Option (a) looks tempting, but wholesale overwriting `/etc/passwd` is dangerous,
as you'll blow away whatever changes the container build script may have made to
the container's `/etc/passwd`. If you're not careful to use a read-only mount,
you'll propagate modifications from the container to the host. Don't forget that
there's no guarantee that the host machine even *has* an `/etc/passwd`, like
when running on Windows. It's also not entirely correct, as adding a user to a
Linux system is more complicated than just updating `/etc/passwd`; handling
filesystem permissions properly requires adding a group named after the user as
well.

autouseradd is an unmentioned option (c) that adds a user and an eponymous group
through the proper channels, resulting in valid `/etc/passwd` and `/etc/group`
files and a writable home directory with no muss or fuss.

[docker-run-user]: https://docs.docker.com/engine/reference/run/#user
[postgres-user-notes]: https://github.com/docker-library/docs/blob/212ed60a19b8b5454308b504fdcdf5d317f42b6f/postgres/README.md#arbitrary---user-notes
