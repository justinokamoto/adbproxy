#ifndef ADBP_TYPES_H
#define ADBP_TYPES_H

enum adbp_transport {
    ADBP_ADB_SERVER_TRANSPORT,
    ADBP_ADBD_REVERSE_TRANSPORT,
    ADBP_ADBD_TRANSPORT
};

enum adbp_request_type {
    ADBP_FORWARD,
    ADBP_FORWARD_KILL,
    ADBP_FORWARD_KILL_ALL,
    ADBP_UNDEFINED
};

typedef struct {
    int valid;
    enum adbp_request_type req_type;
    enum adbp_transport req_transport;
    int local_port;
    int device_port;
} adbp_forward_req;

#endif