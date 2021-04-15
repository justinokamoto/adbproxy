CCFLAGS=

UNAME := $(shell uname)

all: library

debug: CCFLAGS += -DDEBUG
debug: library

ifeq ($(UNAME),Darwin)
library: adbp.c message.c
	clang $(CCFLAGS) -dynamiclib -current_version 1.0 adbp.c message.c -o libadbp.dylib
endif
ifeq ($(UNAME),Linux)
library: adbp.c message.c
	clang $(CCFLAGS) -shared -fPIC adbp.c message.c -o libadbp.so
endif

example: example.c adbp.c message.c
	clang $(CCFLAGS) -o adb_proxy_example example.c adbp.c message.c

test: test/test.c message.c
	clang $(CCFLAGS) -o adb_proxy_tests test/test.c message.c -lcheck
