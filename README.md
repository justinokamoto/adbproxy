# ADB Proxy
ADB Proxy is a simple, unobtrusive TCP proxy that proxies traffic between ADB client and ADB server without modification. Its only purpose is to parse ADB packets to understand when port forwarding commands are being sent. So it is made to understand the following ADB commands:
```
$ adb reverse tcp:<port> tcp:<port>
$ adb forward tcp:<port> tcp:<port>
$ adb forward --remove tcp:<port>
$ adb reverse --remove tcp:<port>
$ adb reverse --remove-all
$ adb forward --remove-all
```
This is useful for scenarios when ADB client and ADB server are running on separate hosts (or even separate network interfaces), where ports between ADB client host and ADB server host will need to be forwarded accordingly in order for the above commands to work as expected.

For example, say we have Host A running ADB client commands and Host B running an ADB server with an Android device attached. If Host A runs `adb reverse tcp:3001 tcp:3002`, it should expect that traffic from the Android device's port `3001` would reach its (Host A's) port `3002`. However, traffic from port `3002` on Host B will need to be forwarded to port `3002` on Host A in order for this to function as expecte\
d.

This is where ADB Proxy is useful, since it can be used to capture port forwarding commands so port forwarding can be setup appropriately (via SSH or whatnot).

For more background, see [this blog post](https://medium.com/@justinchips/proxying-adb-client-connections-2ab495f774eb).

## Installing
ADB Proxy has the following dependencies:
*  [libcheck](https://libcheck.github.io/check/web/install.html)-0.15.2

## Usage
API is straightforward. Start the proxy server by passing proxy port, adb server port (should already be running), and handler function. Then you can write your own custom logic within your handler, like forwarding ports between hosts . See `example.c` for more details.

(TIP: If you intend on forwarding ports via `ssh`, consider [SSH multiplexing](https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Multiplexing) to reduce latency)
```
#include "adbp.h"
#include "adbp_types.h"

void *handler(adbp_forward_req req)
{
    if (req.valid) {
		// Do something!
    }
}

int main(int argc, char const *argv[])
{
    int proxy_port = 5038;
    int adb_server_port = 5037;
    adbp_start_server(proxy_port, adb_server_port, handler);
    return 0;
}
```
## Make
### Build library:
```
$ make # optional 'debug' target for verbose logging
```
### Build and run example:
```
$ make example
$ ./adb_proxy_example # starts proxy on port 5038
```
To test the proxy, you can run adb command against the proxy:
```
$ adb -P 5038 forward --remove-all
```
You should see the example proxy print `FORWARD KILL ALL`

# Resources

ADB client <---> ADB server

*  [Android Documentation](https://android.googlesource.com/platform/packages/modules/adb/+/master/SERVICES.TXT)

ADB server <---> ADBD on device

*  [Android Documentation](https://android.googlesource.com/platform/system/core/+blame/master/adb/protocol.txt)

*  [Unofficial Documentation](https://github.com/cstyan/adbDocumentation)
