CFLAGS=-Os -Wall -Wextra -g
OBJECTS:= \
	out/main.o \
	out/interrupts.o \
	out/govee.o \
	out/bluetooth_eir.o \
	out/scan.o \
	out/bluetooth_entrypoint.o \
	out/prom_entrypoint.o \
	out/govee_device_list.o \
	out/metrics.o \
	out/config.o \
	out/aranet.o
BINARY:=bin/monitor

.PHONY: clean all
all: $(BINARY)
clean::
	rm -rf bin lib out
	mkdir -p out
	mkdir -p bin

out/%.o: src/%.c $(wildcard src/*.h)
	gcc -c $(CFLAGS) -o $@ $<

$(BINARY): $(OBJECTS)
	gcc -g $^ \
		-static \
		-l:libbluetooth.a -lpthread \
		-o $@
