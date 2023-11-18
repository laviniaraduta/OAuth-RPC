RPCGEN = rpcgen
CC = g++
LDFLAGS = -I/usr/include/tirpc -lnsl

all: generate build

generate:
	$(RPCGEN) -h rpc_authentication.x -o rpc_authentication.h
	$(RPCGEN) -m rpc_authentication.x -o rpc_authentication_svc.c
	$(RPCGEN) -l rpc_authentication.x -o rpc_authentication_clnt.c 
	$(RPCGEN) -c rpc_authentication.x -o rpc_authentication_xdr.c

build:
	$(CC) -o server -g  server.cpp authentication_server.cpp resource_server.cpp end_user.cpp rpc_authentication_xdr.c $(LDFLAGS)
	$(CC) -o client -g  client.cpp rpc_authentication_clnt.c rpc_authentication_xdr.c $(LDFLAGS)

pack:
	zip -FSr lavinia_raduta343C1_sprc1.zip Makefile tests/ *.cpp *.h *.x *.c check.sh README.md

clean:
	rm server client rpc_authentication_xdr.c rpc_authentication_clnt.c rpc_authentication_svc.c  rpc_authentication.h