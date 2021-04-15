#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "message.h"
#include <pthread.h>
#include "adbp.h"

void *handler(adbp_forward_req req)
{

}

int main(int argc, char const *argv[])
{
    // TODO: Make this run in its own thread
    adbp_start_server(5038, 5037, handler); // Should be function pointer!
    return 0;
}
