#ifndef ADBP_MESSAGE_H
#define ADBP_MESSAGE_H
#include "adbp_types.h"

void parse_payload(adbp_forward_req *req, char *payload, int payload_len);

#endif