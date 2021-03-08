#include "message.h"
#include <unistd.h>

#include <math.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <regex.h>


#include <stdio.h>
#include <sys/socket.h>

/*
 * Convert digit string of `base` and `len` to base 10 num
 * in base 10
 */ 
unsigned int to_base10(char *d_str, int len, int base)
{
    if (len < 1) {
        return 0;
    }
    char d = d_str[0];
    // chars 0-9 = 48-57, chars a-f = 97-102
    int val = (d > 57) ? d - ('a' - 10) : d - '0';
    int result = val * pow(base, (len - 1));
    d_str++; // increment pointer
    return result + to_base10(d_str, len - 1, base);
}

int parse_content_len(char *payload, int payload_len, int *content_len)
{
    // TODO: Have it detect null termination (and remove it from len)
    int result = to_base10(payload, HEADER_LEN, 16);
    if (result != payload_len - HEADER_LEN) {
        // Unparsable packet (happens if message not intended
        // for ADB server). Set -1 for invalid content length
        content_len = NULL;
        return 0;
    }
    if (content_len != NULL) {
        *content_len = result;
    }
    return HEADER_LEN;
}

// TODO: Don't use regex, just prefix()
// then payload doesn't need null termination
int parse_host_prefix(char *payload_nt, enum adbp_transport *transport)
{
    regex_t regex;
    regmatch_t pm;
    int result;
    result = regcomp(&regex, "^reverse:", REG_EXTENDED);
    if (result != 0) {
        perror("Reverse regex did not compile");
        exit(EXIT_FAILURE);
    }
    result = regexec(&regex, payload_nt, 1, &pm, 0);
    if (result == 0) {
        *transport = ADBP_ADBD_REVERSE_TRANSPORT;
        return pm.rm_eo;
    }
    result = regcomp(&regex, "^host:|host-usb:|host-local:|host-serial:[A-Za-z0-9-]{1,20}:", REG_EXTENDED);
    if (result != 0) {
        perror("Host regex did not compile");
        exit(EXIT_FAILURE);
    }
    result = regexec(&regex, payload_nt, 1, &pm, 0);
    if (result == 0) {
        *transport = ADBP_ADB_SERVER_TRANSPORT;
        return pm.rm_eo;
    }
    // Assume packet is directly for adbp
    *transport = ADBP_ADBD_TRANSPORT;
    return 0;
}

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int parse_forward_type(char *payload_nt, enum adbp_request_type *req_type)
{
    // TODO: Make the 'jumping' of these chars different? ':'
    char forward_prefix[] = "forward";
    char forward_kill_prefix[] = "killforward";
    char forward_kill_all_prefix[] = "killforward-all";
    if (prefix(forward_prefix, payload_nt)) {
        *req_type = ADBP_FORWARD;
        // Don't -1 in order to 'jump' over the ':'
        return sizeof(forward_prefix);
    } else if (prefix(forward_kill_all_prefix, payload_nt)) {
        *req_type = ADBP_FORWARD_KILL_ALL;
        return sizeof(forward_kill_all_prefix) - 1;
    } else if (prefix(forward_kill_prefix, payload_nt)) {
        *req_type = ADBP_FORWARD_KILL;
        return sizeof(forward_kill_prefix);
    } else {
        *req_type = ADBP_UNDEFINED;
        return 0;
    }
}

int parse_next_port_num(char *payload_nt, int *port)
{
    regex_t regex;
    regmatch_t pm;
    int result;
    char prefix[4] = "tcp:"; // not null-terminated
    // TODO: sprintf prefix into this string v
    // TODO: Don't use regex (just prefix())
    result = regcomp(&regex, "^tcp:[[:digit:]]{1,5}", REG_EXTENDED);
    if (result != 0) {
        perror("Regex did not compile");
        exit(EXIT_FAILURE);
    }
    result = regexec(&regex, payload_nt, 1, &pm, 0);
    if (result != 0) {
        *port = 0;
        return 0;
    }
    // match offset - chars in 'tcp:' prefix + null terminating char
    char port_str[pm.rm_eo - sizeof(prefix) + 1];
    memcpy(port_str, payload_nt + sizeof(prefix), sizeof(port_str));
    port_str[sizeof(port_str) - 1] = '\0';

    *port = atoi(port_str);
    if (payload_nt[pm.rm_eo] == ';')
    {
        // Jump over ';', e.g., 'tcp:1000;'
        return pm.rm_eo + 1;
    } else {
        return pm.rm_eo;
    }
}

void parse_payload(adbp_forward_req *req, char *payload, int payload_len) {
    int bytes_parsed = 0;
    // Default req to invalid
    req->valid = 0;
    // Parse content len
    bytes_parsed += parse_content_len(payload, payload_len, NULL);
    if (bytes_parsed == 0) {
        // not valid packet for adb server (probably meant for adbd)
        return;
    }
    // Parse host prefix
    bytes_parsed += parse_host_prefix(payload + bytes_parsed, &(req->req_transport));
    if (req->req_transport == ADBP_ADBD_TRANSPORT) {
        // packet meant for adbd
        return;
    }
    // Parse forwarding type
    bytes_parsed += parse_forward_type(payload + bytes_parsed, &(req->req_type));
    if (req->req_type == ADBP_UNDEFINED) {
        // host/reverse packet contains different request type
        return;
    }
    // Parse ports
    if (req->req_type == ADBP_FORWARD) {
        // Parse out local port
        bytes_parsed += parse_next_port_num(payload + bytes_parsed, &(req->local_port));
        // Parse out device port
        parse_next_port_num(payload + bytes_parsed, &(req->device_port));
    } else if (req->req_type == ADBP_FORWARD_KILL) {
        // Parse out local port
        parse_next_port_num(payload + bytes_parsed, &(req->local_port));
    }
    // If finished parsing, req is valid
    req->valid = 1;
}