# ADB Proxy

AOSP created the Android Device Bridge (ADB) protocol for communication between host and Android device. The protocol is realized by the Android platform tools which provides the ADB client/server utility used to communicate with the ADB daemon (ADBD) that runs on the Android device.

The client/server nature of the ADB utility is useful, since the host intending to communicate with a given Android device does not have to be the same host to which that device is connected. However, there are rare cases where this model is broken, namely, when managing port forwarding between the host and device, as illustrated below:

```
TODO: ASCII ART
```

TODO: Android documentation on possible forwarding/reverse-forwarding commands

ADB proxy is a simple proxy ADB server that will simply passthrough all ADB traffic, while watching for forward messages. When a forward message is found, the proxy will then write this information to a specified socket.

```
TODO: ASCII ART
```

The intention is to be used in tandom with another utility that, when given a port to forward, forwards said port between host executing the ADB client and host running the ADB server. An example below is given which forwards ports using SSH:

```
TODO: Small example
```
