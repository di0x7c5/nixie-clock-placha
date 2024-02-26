#
# Makefile for Nixie Clock by Dawid Placha
# Copyright (C) di0x7c5
#

TARGET   := nixieDP
SRC      := nixieclock.c
F_CPU    := 16000000
OPTIMIZE := -Os

MCU      := atmega328p
#MCU      := atmega8
LFUSE    := 0xff
HFUSE    := 0xdf
EFUSE    := 0xff

# ----------------------------------------------------------------------------
# You should not have to change anything below here.

CC  := avr-gcc
OBJ := $(SRC:.c=.o)

OBJCOPY := avr-objcopy
OBJDUMP := avr-objdump
SIZE := avr-size

# Programming support using avrdude. Settings and variables.
AVRDUDE := avrdude
AVRDUDE_PROGRAMMER := usbasp
AVRDUDE_MCU := m328p
#AVRDUDE_MCU := m8

CFLAGS := -mmcu=$(MCU) -I. -g -Wall -DF_CPU=$(F_CPU) -D$(MCU) $(OPTIMIZE) -std=gnu99

all: build

build: $(TARGET).elf $(TARGET).hex
	$(SIZE) --format=avr --mcu=$(MCU) $(TARGET).elf

clean:
	rm -f $(TARGET).hex $(TARGET).elf $(OBJ) *.o

flash: $(TARGET).hex
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c $(AVRDUDE_PROGRAMMER) -U flash:w:$(TARGET).hex

fuses:
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c $(AVRDUDE_PROGRAMMER) -v -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

.SUFFIXES: .elf .hex .eep .lss .sym

.elf.hex:
	$(OBJCOPY) -O ihex -R .eeprom $< $@

.elf.eep:
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

.elf.lss:
	$(OBJDUMP) -h -S $< > $@

.elf.sym:
	$(NM) -n $< > $@

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) --output $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

.c.s:
	$(CC) -S $(CFLAGS) $< -o $@

.S.o:
	$(CC) -c $(AFLAGS) $< -o $@

.PHONY:	help all build clean flash fuses
