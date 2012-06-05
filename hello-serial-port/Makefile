
NAME := hello-serial-port

PARTS = hex.o main.o

include ../support/Cortex-M3.mk

DELIVERABLES := PAGE30.HEX

all: hexes

hexes: $(DELIVERABLES)
	if [ -e PAGE30.elf ]; then $(SIZE) PAGE30.elf; fi

