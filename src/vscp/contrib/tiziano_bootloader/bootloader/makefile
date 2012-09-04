#######################################################
#
# Makefile for LPC2368 
#
#######################################################

#
# Target Setup
#
TARGETNAME=bootloader
TARGETDIR = ./

#
# Linkerscript setup
#
LINKERSCRIPT = ./prj/lpc2364_rom.ld

#
# Source Setup
#
#  Add a line to the rules for each source file
#   to compile it
#
SOURCEDIR = ./src

#
# Objects setup
#
#  Add object name for each each source file
#
OBJECTDIR = $(SOURCEDIR)

COBJECTS  = $(OBJECTDIR)/main.o $(OBJECTDIR)/handlers.o $(OBJECTDIR)/init_cpu.o
COBJECTS += $(OBJECTDIR)/led.o $(OBJECTDIR)/vic.o
COBJECTS += $(OBJECTDIR)/utils.o $(OBJECTDIR)/uart.o 
COBJECTS += $(OBJECTDIR)/key.o $(OBJECTDIR)/evt.o
COBJECTS += $(OBJECTDIR)/buff.o $(OBJECTDIR)/tmr.o
COBJECTS += $(OBJECTDIR)/flash.o $(OBJECTDIR)/upload.o
COBJECTS += $(OBJECTDIR)/ssp.o $(OBJECTDIR)/spiflash.o
COBJECTS += $(OBJECTDIR)/can.o $(OBJECTDIR)/vscp.o
COBJECTS += $(OBJECTDIR)/vscp_main.o $(OBJECTDIR)/vscp_events.o
COBJECTS += $(OBJECTDIR)/i2c.o $(OBJECTDIR)/emem.o
COBJECTS += $(OBJECTDIR)/crc.o


#
# Startarm Setup
#
STARTARM  =./src/startarm.s

#
# Include directories
#
INCLUDEDIRS = -I./inc/

#
# Tools Setup
#
CC        =arm-elf-gcc
OBJCOPY   =arm-elf-objcopy

#
# CFLAGS
#
WARNINGS   =-Wall -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
		-Waggregate-return -Wstrict-prototypes -Wunused
#CPU_TYPE   = -mlittle-endian -mcpu=arm7tdmi -marm -fpack-struct -DUART1_LPRINTF
CPU_TYPE   = -mlittle-endian -mcpu=arm7tdmi -marm -DUART1_LPRINTF
# DEBUGFLAGS = -gdwarf-2 -MD 
DEBUGFLAGS = -gdwarf-2 -MD -O
# DEBUGFLAGS = -g -gdwarf-2

CFLAGS     =$(WARNINGS) $(INCLUDEDIRS) $(DEBUGFLAGS) $(CPU_TYPE) -T$(LINKERSCRIPT)
CXFLAGS     =$(WARNINGS) $(INCLUDEDIRS) -gdwarf-2 -MD $(CPU_TYPE) -T$(LINKERSCRIPT)

#
# LINKERFLAGS
#
LINKERFLAGS=-Xlinker -o$(TARGETDIR)/$(TARGETNAME).elf -Xlinker -M -Xlinker -Map=$(TARGETDIR)/$(TARGETNAME).map

#
# FOR DEVICE PROGRAMMING
#
IMGEXT = bin

#######################################################
#
# Rules
#
#######################################################

#
# all
# 
all: $(TARGETDIR)/$(TARGETNAME).hex $(TARGETDIR)/$(TARGETNAME).bin


#
# C files
#
#  Add extra lines for each source file
#
$(OBJECTDIR)/main.o : $(SOURCEDIR)/main.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTDIR)/handlers.o : $(SOURCEDIR)/handlers.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTDIR)/init_cpu.o : $(SOURCEDIR)/init_cpu.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTDIR)/led.o : $(SOURCEDIR)/led.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/vic.o : $(SOURCEDIR)/vic.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/utils.o : $(SOURCEDIR)/utils.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/uart.o : $(SOURCEDIR)/uart.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/key.o : $(SOURCEDIR)/key.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/evt.o : $(SOURCEDIR)/evt.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/buff.o : $(SOURCEDIR)/buff.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@
	
$(OBJECTDIR)/tmr.o : $(SOURCEDIR)/tmr.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/flash.o : $(SOURCEDIR)/flash.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/upload.o : $(SOURCEDIR)/upload.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/ssp.o : $(SOURCEDIR)/ssp.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/spiflash.o : $(SOURCEDIR)/spiflash.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/can.o : $(SOURCEDIR)/can.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@
	
$(OBJECTDIR)/vscp.o : $(SOURCEDIR)/vscp.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@
	
$(OBJECTDIR)/vscp_main.o : $(SOURCEDIR)/vscp_main.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTDIR)/vscp_events.o : $(SOURCEDIR)/vscp_events.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@
	
$(OBJECTDIR)/i2c.o : $(SOURCEDIR)/i2c.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTDIR)/emem.o : $(SOURCEDIR)/emem.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
$(OBJECTDIR)/crc.o : $(SOURCEDIR)/crc.c $(LINKERSCRIPT) Makefile
	$(CC) -c $(CFLAGS) $< -o $@	
	
#
# Elf file
#
$(TARGETDIR)/$(TARGETNAME).elf : $(COBJECTS) $(STARTARM) Makefile
	$(CC) $(CFLAGS) $(COBJECTS) -nostartfiles $(STARTARM) $(LINKERFLAGS)

#
# Binary file
#
$(TARGETDIR)/$(TARGETNAME).bin : $(TARGETDIR)/$(TARGETNAME).elf
	$(OBJCOPY) $(TARGETDIR)/$(TARGETNAME).elf -O binary $(TARGETDIR)/$(TARGETNAME).bin

#
# Hex file
#
$(TARGETDIR)/$(TARGETNAME).hex : $(TARGETDIR)/$(TARGETNAME).elf
	$(OBJCOPY) $(TARGETDIR)/$(TARGETNAME).elf -O ihex $(TARGETDIR)/$(TARGETNAME).hex
	$(CC) --version

#	
# Program the device with Dominic Rath's OPENOCD in "batch-mode", needs cfg and "reset-script".
#
# program: $(TARGETNAME).$(IMGEXT)
# 	@echo
# 	@echo "Programming with OPENOCD"
# 	.\prj\openocd_go_flash.cmd

#
# Clean
#
clean :
	rm -f $(OBJECTDIR)/*.o
	rm -f $(OBJECTDIR)/*.d
	rm -f $(TARGETDIR)/*.elf
	rm -f $(TARGETDIR)/*.hex
	rm -f $(TARGETDIR)/*.bin
	rm -f $(TARGETDIR)/*.map



