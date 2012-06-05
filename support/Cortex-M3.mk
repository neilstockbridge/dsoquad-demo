
.SILENT:

# The directory for .o files and such.  Keep it in RAM for speed and to reduce
# flash disc wear
TDIR = /tmp/$(NAME)

# Names of the toolchain programs
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CFLAGS += -mcpu=cortex-m3 -mthumb -mno-thumb-interwork # Processor type
CFLAGS += -fno-common -Os -std=gnu99 $(DEBUG) -ffunction-sections # Optimization and debug settings
CFLAGS += -Wall -Wno-unused # Compiler warnings
CFLAGS += -I../support -I../lib # so that make can find the DSO Quad platform files

LFLAGS += -L../support # helps the linker find the .lds files
LFLAGS += -nostartfiles -Wl,-Map=$(TDIR)/$(NAME).map -eReset_Handler -Wl,-gc-sections # Disables GCC-provided startup.c, creates .map file

# Add the support files to the list of objects to build:
PARTS += startup.o BIOS.o

# Tell make where to find referenced files that are not in the current directory:
VPATH = ../support:../lib:$(TDIR):$(VPATH_EXTRA)

# How to make .HEX files from .elf files:
%.HEX: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.elf: %.lds $(PARTS) | $(TDIR)
	$(CC) $(CFLAGS) -o $@ $(PARTS) $(LFLAGS) $(LIBS) -T $<

# Rebuild all parts if any header or the Makefile changes:
.c.o: *.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

.S.o: | $(TDIR)
	$(CC) $(AFLAGS) -c -o $@ $<

firsttarget: all

$(TDIR):
	mkdir $(TDIR)

clean:
	rm -f $(TDIR)/*.o $(TDIR)/*.map *.elf $(DELIVERABLES) $(CLEAN)

