#include "../message.h"

#include <check.h>
#include <stdlib.h>
#include <fcntl.h>
#include <regex.h>

START_TEST (test_parse_content_len_unparsable)
{
    int content_len = 0;
    char payload[] = "deadbeef";
    int bytes_read = parse_content_len(&payload, sizeof(payload) - 1, &content_len);
    ck_assert_int_eq(bytes_read, 0);
    ck_assert_int_eq(content_len, 0);
}
END_TEST

START_TEST (test_parse_content_len)
{
    int content_len;
    char payload[] = "000chost:version";
    int bytes_read = parse_content_len(&payload, sizeof(payload) - 1, &content_len);
    ck_assert_int_eq(bytes_read, 4);
    ck_assert_int_eq(content_len, 12);
}
END_TEST

START_TEST (test_parse_host_prefix)
{
    enum adbp_transport transport;

    // Test host transport
    ck_assert_int_eq(parse_host_prefix(&"host:forward:tcp:10000;tcp:10001", &transport), 5);
    ck_assert_int_eq(transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(parse_host_prefix(&"host-usb:forward:tcp:10000;tcp:10001", &transport), 9);
    ck_assert_int_eq(transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(parse_host_prefix(&"host-local:forward:tcp:10000;tcp:10001", &transport), 11);
    ck_assert_int_eq(transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(parse_host_prefix(&"host-serial:SERIAL-NUM123:forward:tcp:10000;tcp:10001", &transport), 26);
    ck_assert_int_eq(transport, ADBP_ADB_SERVER_TRANSPORT);

    // Test adbd reverse cases
    ck_assert_int_eq(parse_host_prefix(&"reverse:killforward-all", &transport), 8);
    ck_assert_int_eq(transport, ADBP_ADBD_REVERSE_TRANSPORT);

    // Test adbd cases
    ck_assert_int_eq(parse_host_prefix(&"0020host:forward:tcp:10000;tcp:10001", &transport), 0);
    ck_assert_int_eq(transport, ADBP_ADBD_TRANSPORT);
}
END_TEST

START_TEST (test_parse_forward_type)
{
    enum adbp_request_type req;
    ck_assert_int_eq(parse_forward_type(&"forward:tcp:10000;tcp:10001", &req), 8);
    ck_assert_int_eq(req, ADBP_FORWARD);

    ck_assert_int_eq(parse_forward_type(&"killforward:10000", &req), 12);
    ck_assert_int_eq(req, ADBP_FORWARD_KILL);

    ck_assert_int_eq(parse_forward_type(&"killforward-all", &req), 15);
    ck_assert_int_eq(req, ADBP_FORWARD_KILL_ALL);

    ck_assert_int_eq(parse_forward_type(&"fireworks:invalid:format", &req), 0);
    ck_assert_int_eq(req, ADBP_UNDEFINED);
}
END_TEST

START_TEST (test_parse_next_port_num)
{
    int port;
    int bytes_read;
    bytes_read = parse_next_port_num(&"tcp:10000;", &port);
    ck_assert_int_eq(bytes_read, 10);
    ck_assert_int_eq(port, 10000);
    bytes_read = parse_next_port_num(&"tcp:10000", &port);
    ck_assert_int_eq(bytes_read, 9);
    ck_assert_int_eq(port, 10000);
}
END_TEST

START_TEST (test_parse_payload)
{
    adbp_forward_req req;
    // Test forward port
    char p1[] = "0020host:forward:tcp:10000;tcp:10001";
    parse_payload(&req, &p1, sizeof(p1) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD);
    ck_assert_int_eq(req.local_port, 10000);
    ck_assert_int_eq(req.device_port, 10001);

    // Test reverse forward port
    char p2[] = "0021reverse:forward:tcp:3000;tcp:3001";
    parse_payload(&req, &p2, sizeof(p2) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADBD_REVERSE_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD);
    ck_assert_int_eq(req.local_port, 3000);
    ck_assert_int_eq(req.device_port, 3001);

    // Test forward port kill
    char p3[] = "001ahost:killforward:tcp:10000";
    parse_payload(&req, &p3, sizeof(p3) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD_KILL);
    ck_assert_int_eq(req.local_port, 10000);  

    // Test reverse forward port kill
    char p4[] = "001creverse:killforward:tcp:3001";
    parse_payload(&req, &p4, sizeof(p4) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADBD_REVERSE_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD_KILL);
    ck_assert_int_eq(req.local_port, 3001);

    // Test forward kill-all
    char p5[] = "0017reverse:killforward-all";
    parse_payload(&req, &p5, sizeof(p5) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADBD_REVERSE_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD_KILL_ALL);

    // Test reverse kill-all
    char p6[] = "0014host:killforward-all";
    parse_payload(&req, &p6, sizeof(p6) - 1);
    ck_assert_int_eq(req.valid, 1);
    ck_assert_int_eq(req.req_transport, ADBP_ADB_SERVER_TRANSPORT);
    ck_assert_int_eq(req.req_type, ADBP_FORWARD_KILL_ALL);

    // Test invalid bit set for wrong content length
    char p7[] = "0030host:forward:tcp:10000;tcp:10001";
    parse_payload(&req, &p7, sizeof(p7) - 1);
    ck_assert_int_eq(req.valid, 0);
}
END_TEST

Suite *adb_proxy_suite(void)
{
    Suite *s;
    TCase *tc_core;
    TCase *tc_limits;

    s = suite_create("AdbProxy");
    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_parse_content_len);
    tcase_add_test(tc_core, test_parse_content_len_unparsable);
    tcase_add_test(tc_core, test_parse_host_prefix);
    tcase_add_test(tc_core, test_parse_forward_type);
    tcase_add_test(tc_core, test_parse_next_port_num);
    tcase_add_test(tc_core, test_parse_payload);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = adb_proxy_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
