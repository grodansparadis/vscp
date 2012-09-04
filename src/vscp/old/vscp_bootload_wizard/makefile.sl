# Purpose: makefile for vscpboot example (Salford C++)
# Created: 2000-03-15

PROGRAM = vscpboot
OBJECTS = $(PROGRAM).obj

include ..\..\src\makeprog.sl

all:        wx $(TARGET)

wx:
    cd $(WXDIR)\src\msw ^ mk32 -f makefile.sl all
    cd $(WXDIR)\samples\vscpboot

