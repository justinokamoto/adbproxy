
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

For example, say we have Host A running ADB client commands and Host B running an ADB server with an Android device attached. If Host A runs `adb reverse tcp:3001 tcp:3002`, it should expect that traffic from the Android device's port `3001` would reach its (Host A's) port `3002`. However, traffic from port `3002` on Host B will need to be forwarded to port `3002` on Host A in order for this to function as expected.

This is where ADB Proxy is useful, since it can be used to capture port forwarding commands so port forwarding can be setup appropriately (via SSH or whatnot).

For more background, see [this blog post](https://medium.com/@justinchips/proxying-adb-client-connections-2ab495f774eb).

## TODO: Usage as library
TODO: Example 
```
$ adb reverse tcp:3001 tcp:3002
$ nc -l localhost 3002 & # listen on 3002
$ adb shell nc localhost 3001 # fails, as this port is only being forwarded on Host B
```  
## Installing
ADB Proxy has the following dependencies:
*  [libcheck](https://libcheck.github.io/check/web/install.html)-0.15.2

## Make
Make and run proxy executable:
```
$ make # optional 'debug' target
$ ./adb_proxy
```
Make and run test executable:
```
$ make test
$ ./adb_proxy_tests
```
# Resources

ADB client <---> ADB server

*  [Android Documentation](https://android.googlesource.com/platform/packages/modules/adb/+/master/SERVICES.TXT)

ADB server <---> ADBD on device

*  [Android Documentation](https://android.googlesource.com/platform/system/core/+blame/master/adb/protocol.txt)

*  [Unofficial Documentation](https://github.com/cstyan/adbDocumentation)
