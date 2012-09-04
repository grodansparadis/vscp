#################################################################
## This Makefile Exported by MinGW Developer Studio
## Copyright (c) 2002-2004 by Parinya Thipchart
#################################################################

ifneq (,$(findstring Release, $(CFG)))
  override CFG = Release
else
  override CFG = Debug
endif

PROJECT = wxPieCtrlTest
CC = g++

ifeq ($(CFG),Debug)
  OBJ_DIR = Debug
  OUTPUT_DIR = Debug
  TARGET = wxPieCtrlTest
  C_INCLUDE_DIRS = 
  C_PREPROC = 
  CFLAGS = -pipe `wx-config --cxxflags` -I../../include -Wall -g2 -O0 -fno-rtti -fno-exceptions 
  LIB_DIRS = 
  LIBS = 
  LDFLAGS = -pipe `wx-config --libs`-l../../lib/libwxPieCtrl.a
endif

ifeq ($(CFG),Release)
  OBJ_DIR = Release
  OUTPUT_DIR = Release
  TARGET = wxPieCtrlTest
  C_INCLUDE_DIRS = 
  C_PREPROC = 
  CFLAGS = -pipe `wx-config --cxxflags`  -Wall -g0 -O2 -fno-rtti -fno-exceptions 
  LIB_DIRS = 
  LIBS = 
  LDFLAGS = -pipe -s `wx-config --libs`
endif

SRC_OBJS = \
  $(OBJ_DIR)/wxPieCtrlTest.o

define build_target
@echo Linking...
@$(CC) -o "$(OUTPUT_DIR)/$(TARGET)" $(SRC_OBJS) $(LIB_DIRS) $(LIBS) $(LDFLAGS)
endef

define compile_source
@echo Compiling $<
@$(CC) $(CFLAGS) $(C_PREPROC) $(C_INCLUDE_DIRS) -c "$<" -o "$@"
endef

.PHONY: print_header directories

$(TARGET): print_header directories $(SRC_OBJS)
	$(build_target)

.PHONY: clean cleanall

cleanall:
	@echo Deleting intermediate files for 'wxPieCtrlTest - $(CFG)'
	-@rm -rf "$(OBJ_DIR)"
	-@rm -rf "$(OUTPUT_DIR)/$(TARGET)"
	-@rmdir "$(OUTPUT_DIR)"

clean:
	@echo Deleting intermediate files for 'wxPieCtrlTest - $(CFG)'
	-@rm -rf $(OBJ_DIR)

print_header:
	@echo ----------Configuration: wxPieCtrlTest - $(CFG)----------

directories:
	-@if [ ! -d "$(OUTPUT_DIR)" ]; then mkdir "$(OUTPUT_DIR)"; fi
	-@if [ ! -d "$(OBJ_DIR)" ]; then mkdir "$(OBJ_DIR)"; fi

$(OBJ_DIR)/wxPieCtrlTest.o: wxPieCtrlTest.cpp

	$(compile_source)



