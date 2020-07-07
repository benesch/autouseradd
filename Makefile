PREFIX ?= /usr/local
BINDIR = $(DESTDIR)$(PREFIX)/bin
LIBEXECDIR = $(DESTDIR)$(PREFIX)/libexec
MANDIR = $(DESTDIR)$(PREFIX)/share/man

CFLAGS += -Wall -Wextra -Werror -O2 -DPREFIX=$(PREFIX)

all: autouseradd autouseradd-suid

install: autouseradd autouseradd-suid
	install -d $(BINDIR) $(LIBEXECDIR) $(MANDIR)/man1
	install -o root -g root -m u=rwx,g=rx,o=rx autouseradd $(BINDIR)/autouseradd
	install -o root -g root -m u=rwxs,g=rxs,o=rx autouseradd-suid $(LIBEXECDIR)/autouseradd-suid
	install -m u=rw,g=r,o=r autouseradd.1 $(MANDIR)/man1/autouseradd.1

check:
	docker build -f test-container.docker -t autouseradd .
	bats test.bats $(CHECKARGS)

.PHONY: all install check
