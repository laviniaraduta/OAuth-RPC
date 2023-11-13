RPC = rpcgen
CC = g++
LDFLAGS = -I/usr/include/tirpc -lnsl

all: generate build

generate:
	$(RPC) -m rpc_authentication.x -o rpc_authentication_svc.c
	$(RPC) -l rpc_authentication.x -o rpc_authentication_clnt.c 
	$(RPC) -c rpc_authentication.x -o rpc_authentication_xdr.c
	$(RPC) -h rpc_authentication.x -o rpc_authentication.h

build:
	$(CC) -o server -g  server.cpp rpc_authentication_svc.c rpc_authentication_xdr.c $(LDFLAGS)
	$(CC) -o client -g  client.cpp rpc_authentication_clnt.c rpc_authentication_xdr.c $(LDFLAGS)

clean:
	rm server client rpc_authentication_xdr.c rpc_authentication_clnt.c rpc_authentication_svc.c  rpc_authentication.h