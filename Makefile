# ******************************************************************************
# Header
#
# Project:	setblocksize
# This file:	Makefile
#
# Description:	Makefile to build setblocksize
#
# Copyright:    (C) 2002 by Michael Baeuerle
# License:	GPL V2 or any later version
#
# Language:	GNU make
# Style rules:	-
#
# Written for:	Platform:	all
#		OS:		UNIX
# Tested with:	Interpreter:	GNU make (Version 3.77)
#		Platform:	IA32 (Pentium)
# 		OS:		GNU/Linux (Kernel version: 2.2.10)
# Tested with:	Interpreter:	GNU make (Version 3.77)
#		Platform:	IA32-SMP (2x PentiumPro)
# 		OS:		GNU/Linux (Kernel version: 2.2.17)
# Tested with:  Compiler:       gcc (Version: 3.3.6)
#               Platform:       IA32 (PentiumPro)
#               OS:             GNU/Linux (Kernel version: 2.6.16.20)
# Do not work:  Platform:       non GNU/Linux
# 
# Changelog:
#
# 2002-01-16 by Michael Baeuerle
# Initial version		
#
# 2003-03-22 by Michael Baeuerle
# Minor changes   
#
#
# To do:    -	
# ******************************************************************************


# ******************************************************************************
# Makro definitions
# ******************************************************************************

# Version
VER             = V0.2

# Target names
TARGET		= setblocksize

# Compilers
C1		= gcc

# Flags 
C1FLAGS		= -I $(INCLUDE) -D_REENTRANT -Wall -pipe
A1FLAGS		=
A2FLAGS		=

# Include file and library directory
INCLUDE		= ./include
LIBDIR		= ./lib


# ******************************************************************************
# Main rules
# ******************************************************************************

# Create binary  
all: $(TARGET) 
	@echo
	@echo "Finished."
	@echo

# Delete all object and binary files 
#  (this forces a complete re-build for "make all")
clean:
	@echo
	@echo "Deleting all binary object and archive files ..."
	-rm -f ./*.o
	-rm -f ./$(TARGET)
	-rm -f ../$(TARGET)-$(VER).tar
	@echo

# Create archive of the sources
dist: clean
	@echo "Creating archive of sources ..."
	tar -C . -cvf ../$(TARGET)-$(VER).tar ./* 
	@echo


# ******************************************************************************
# Sub rules
# ******************************************************************************

$(TARGET): $(TARGET).o sg_err.o
	@echo
	@echo "Creating binary ..."
	$(C1) -o $(TARGET) $(TARGET).o sg_err.o

$(TARGET).o: $(TARGET).c
	@echo
	@echo "Creating main object file ..."
	$(C1) $(C1FLAGS) -c -o $(TARGET).o $(TARGET).c

sg_err.o: sg_err.c
	@echo
	@echo "Creating error handling object file ..."
	$(C1) $(C1FLAGS) -c -o sg_err.o sg_err.c


# EOF

