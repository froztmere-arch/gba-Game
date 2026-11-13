PREFIX ?= arm-none-eabi-
CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
CFLAGS  = -mcpu=arm7tdmi -mthumb -mthumb-interwork -O2 -Wall -ffreestanding \
          -fno-builtin -fno-tree-loop-distribute-patterns \
          -fno-unwind-tables -fno-asynchronous-unwind-tables

all: tinyfarm.gba

crt0.o: crt0.s
	$(CC) -mcpu=arm7tdmi -marm -mthumb-interwork -c $< -o $@
main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@
tinyfarm.elf: crt0.o main.o link.ld
	$(CC) -nostartfiles -nostdlib -mthumb-interwork -T link.ld crt0.o main.o -o $@
tinyfarm.gba: tinyfarm.elf
	$(OBJCOPY) -O binary $< $@
	-gbafix $@ || python3 fixheader.py $@

clean:
	rm -f *.o *.elf *.gba
