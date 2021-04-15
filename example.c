#include "adbp.h"

void *handler(adbp_forward_req req)
{
    if (req.valid) {
        if (req.req_transport == ADBP_ADBD_REVERSE_TRANSPORT) {
            printf("REVERSE ");
        }
        if (req.req_type == ADBP_FORWARD) {
            printf("FORWARD ");
        } else if (req.req_type == ADBP_FORWARD_KILL) {
            printf("FORWARD KILL ");
        } else if (req.req_type == ADBP_FORWARD_KILL_ALL) {
            printf("FORWARD KILL ALL ");
        }
        if (req.req_type == ADBP_FORWARD) {
            printf("%d:%d", req.local_port, req.device_port);
        } else if (req.req_type == ADBP_FORWARD_KILL) {
            printf("%d", req.local_port);
        }
        printf("\n");
    }
}

int main(int argc, char const *argv[])
{
    int proxy_port = 5038;
    int adb_server_port = 5037;
    adbp_start_server(proxy_port, adb_server_port, handler);
    return 0;
}
