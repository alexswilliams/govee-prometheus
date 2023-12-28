CFLAGS=-Os -Wall
OBJECTS:= \
	out/main.o \
	out/interrupts.o \
	out/govee.o \
	out/bluetooth_eir.o \
	out/scan.o
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
	gcc $^ \
		-L/usr/lib/x86_64-linux-gnu -lbluetooth \
		-o $@
