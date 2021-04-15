CCFLAGS=

all: executable

debug: CCFLAGS += -DDEBUG
debug: executable

executable: main.c adbp.c message.c
	clang $(CCFLAGS) -o adb_proxy main.c adbp.c message.c

test: test/test.c message.c
	clang $(CCFLAGS) -o adb_proxy_tests test/test.c message.c -lcheck
