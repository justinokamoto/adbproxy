#ifndef ADBP_H
#define ADBP_H
#include "message.h" // Should this be imported?

void adbp_start_server(int proxy_port, int adb_server_port, void (*msg_handler)(adbp_forward_req));
#endif