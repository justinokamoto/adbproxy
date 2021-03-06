#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#include "message.h"

#ifdef DEBUG
 #define D if(1) 
#else
 #define D if(0) 
#endif

// TODO: Refactor into library
// TODO: Better closing of FDs
// TODO: Make this multithreaded (for multiple clients)

// NOTE: 12332 is max, and +1 to null-terimate
// (allowing use of routines requiring strings)
// TODO: This was consistently largest packet len,
// but _not_ definitive
#define MAX_ADB_SERVER_PAYLOAD 16333

struct connection_args {
    int inbound_fd;
    int outbound_fd;
    void (*handler)(adbp_forward_req);
};

// TODO: NARROW DOWN THE INCLUDES

int _bind(int port, struct sockaddr_in *address, int attrlen)
{
    int fd;
    int opt = 1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
	    perror("socket failed");
	    exit(EXIT_FAILURE);
    }
    
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt"); 
        exit(EXIT_FAILURE);
    }
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)address,
                                 attrlen)<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int _connect(int port, struct sockaddr_in *address, int addrlen)
{
    int fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);
    if (connect(fd, (struct sockaddr *) address, addrlen) < 0) {
        perror("socket connection failed");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int process_payload(int bytes_read, char *payload, int payload_len)
{
    if (bytes_read == 0) {
        D printf("Received TCP FIN.\n");
        return 1;
    }
    if (bytes_read == -1) {
        D printf("Received TCP error...Socket is likely closed.\n");
        return 1;
    }
    D printf("TCP payload: '%.*s'\n", payload_len, payload);
    return 0;
}

void *client_inbound(void *vargp)
{
    int direct_transport = 0;
    struct connection_args *args = (struct connection_args *)vargp;

    char payload[MAX_ADB_SERVER_PAYLOAD];
    while(1)
    {
        adbp_forward_req req;
        ssize_t bytes_read = read(args->inbound_fd, &payload, sizeof(payload));
        payload[bytes_read] = '\0'; // null terminate payload string
        D printf("Received client payload.\n");
        if (process_payload(bytes_read, &payload, bytes_read)) {
            break;
        }
        parse_payload(&req, &payload, bytes_read);
        args->handler(req);
        send(args->outbound_fd, &payload, bytes_read, 0);
    }

    close(args->outbound_fd);
    close(args->inbound_fd); 
    pthread_exit(NULL);
}

void adbp_start_server(int proxy_port, int adb_server_port, void (*handler)(adbp_forward_req))
{
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int proxy_fd = _bind(proxy_port, &address, addrlen);
    if (listen(proxy_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    D printf("Starting server on port %d...\n", proxy_port);
    while (1) {
        D printf("Accepting connections...\n");
	    // accept_inbound(proxy_fd, adb_server_port, (struct sockaddr *)&address, (socklen_t *) &addrlen);

        int inbound_socket, valread;
        if ((inbound_socket = accept(proxy_fd, &address, &addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in adb_address;
        int outbound_socket = _connect(adb_server_port, &adb_address, sizeof(adb_address));

        struct connection_args args; // Can this pop from the heap too early? No
        args.inbound_fd = inbound_socket;
        args.outbound_fd = outbound_socket;
        args.handler = handler;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_inbound, (void *) &args);

        char payload[MAX_ADB_SERVER_PAYLOAD];
        while(1)
        {
            ssize_t bytes_read = read(args.outbound_fd, &payload, sizeof(payload));

            D printf("Received server payload.\n");
            if (process_payload(bytes_read, &payload, sizeof(payload))) {
                break;
            }
            send(args.inbound_fd, &payload, bytes_read, 0);
        }

        pthread_cancel(thread_id);
        close(args.inbound_fd);
        close(args.outbound_fd);
    }
}
