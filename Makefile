PREFIX ?= /usr/local
BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(PREFIX)/share/man

CFLAGS += -Wall -Wextra -Werror -O2

all: autouseradd

install: autouseradd
	install -d $(BINDIR) $(MANDIR)/man1
	install -o root -g root -m u=rwxs,g=rxs,o=rx $< $(BINDIR)/$<
	install -m u=rw,g=r,o=r autouseradd.1 $(MANDIR)/man1/autouseradd.1

check:
	docker build -f test-container.docker -t autouseradd .
	bats test.bats

.PHONY: all install check
