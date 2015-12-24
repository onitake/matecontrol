HOST_CC = $(CC)
HOST_LD = $(CC)
HOST_CFLAGS = -O0 -g -Wall -Werror

all: testrb testcurrency

test: all
	./testrb
	./testcurrency

clean:
	rm -rf testrb testcurrency *.o

testmem: testmem.o
	$(HOST_LD) $(HOST_LDFLAGS) -o $@ $^

testrb: testrb.o
	$(HOST_LD) $(HOST_LDFLAGS) -o $@ $^

testcurrency: testcurrency.o
	$(HOST_LD) $(HOST_LDFLAGS) -o $@ $^

%.o: %.c
	$(HOST_CC) $(HOST_CFLAGS) -o $@ -c $<
