#ifndef ADBP_H
#define ADBP_H
#include "adbp_types.h"

void adbp_start_server(int proxy_port, int adb_server_port, void (*handler)(adbp_forward_req));

// TODO: shutdown

#endif