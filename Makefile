CC=gcc
CFLAGS=
LDFLAGS=-luuid -lsodium -lpthread
EXEC=my_secure_chat
PREFIX ?= /usr/local
 
ifeq (${USER},root)
	USERDIR = /home/${SUDO_USER}
	USERNAME=${SUDO_USER}
else
	USERDIR = /home/${USER}
	USERNAME = ${USER}
endif



all: $(EXEC)

my_secure_chat: chat.o main.o client2.o serveur.o parsing.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: chat.h

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

install:
	mkdir -p $(PREFIX)/bin
	cp my_secure_chat $(PREFIX)/bin
	mkdir -p $(USERDIR)/my_secure_chat/conv
	mkdir -p $(USERDIR)/my_secure_chat/keys
	mkdir -p $(USERDIR)/my_secure_chat/dl
	touch $(USERDIR)/my_secure_chat/you
	touch $(USERDIR)/my_secure_chat/userlist
	touch $(USERDIR)/my_secure_chat/seveur.log
	chown -R $(USERNAME):$(USERNAME) $(USERDIR)/my_secure_chat
	chmod -R 774 $(USERDIR)/my_secure_chat

uninstall:
	rm $(PREFIX)/bin/my_secure_chat
