#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#define PROXY_PORT 5037
#define ADB_PORT 5038

#define ADB_PACKET_SIZE 65535

// TODO: Rename using inbound/outbound
// https://android.googlesource.com/platform/system/core/+blame/master/adb/protocol.txt
// https://github.com/cstyan/adbDocumentation

// We're ignoring the messages, since we're only interested in parsing
// payloads that start with specified prefix.

// TODO: Make buffer readonly?
bool matches(char (&buffer) [ADB_PACKET_SIZE]) {
    // TODO: REGEX forwarding and reverse forwarding ports
    // for killing forwarded ports
    // host(-serial):(possible id* +:)(kill)forward(-all)
    // if (buffer.rfind("prefix", 0) == 0) {
    // 	return true;
    // }
    // return false
    return true;
}

void send_outbound_payload(char (&payload) [ADB_PACKET_SIZE], int payload_len)
{
    int outbound_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    if ((outbound_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	perror("outbound socket failed");
	exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( ADB_PORT );
    if (connect(outbound_socket, (struct sockaddr *)&address, addrlen) < 0) {
	perror("outbound socket connection failed");
	exit(EXIT_FAILURE);
    }
    send(outbound_socket, &payload, payload_len, 0);
}

void accept_payload(int server_fd, struct sockaddr *address, socklen_t *addrlen)
{
    int new_socket, valread;
    char buffer[ADB_PACKET_SIZE] = {0};
    if ((new_socket = accept(server_fd, address, addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    } else {
	valread = read(new_socket, buffer, ADB_PACKET_SIZE);
	if (matches(buffer)) {
	    // TODO:
	    // PIPE TO SOMEWHERE WE WANT TO PIPE
	    // PASS ON TO ACTUAL ADB SERVER
	    // SHUTTLE RESPONSE BACK TO REQUESTER
	}
	send_outbound_payload(buffer, valread);
	printf("PAYLOAD: %s\n", buffer);
    }
}

int main(int argc, char const *argv[])
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
	perror("socket failed");
	exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt"); 
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PROXY_PORT );

    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (true) {
	accept_payload(server_fd, (struct sockaddr *)&address, (socklen_t *) &addrlen);
    }
    return 0;
}
