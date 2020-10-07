CROSS_COMPILE?=mips-linux-gnu-
GCC_POSTFIX?=-8

# Target MACH: MALTA or MACH_MT7688
MACH=MALTA

CC=     $(CROSS_COMPILE)gcc$(GCC_POSTFIX)
LD=     $(CROSS_COMPILE)ld

OUTPUTFILE=uart
INCLUDES += -I./h 
OBJDIR = ./obj

CFLAGS += -G0 -O0 -g3 -march=mips32  -Wall
CFLAGS += -mno-abicalls
CFLAGS += -nodefaultlibs
CFLAGS += -fno-pic
CFLAGS += -fno-builtin

ifeq ($(MACH),MALTA)
CFLAGS += -EB
DEFINES=-D__BE__
DEFINES+=-DMACH_MALTA
endif

ifeq ($(MACH),MACH_MT7688)
CFLAGS += -EL
DEFINES=-D__LE__
DEFINES+=-DMACH_MT7688
endif

LDSCRIPT = $(OBJDIR)/linker.lds
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS +=-nostdlib

VPATH =./src

OBJ = startup.o
OBJ += init.o
OBJ += uart.o
OBJ += cbuf.o

COMMON_OBJ =  $(addprefix $(OBJDIR)/, $(OBJ))

###############################################################################
#
# Buld linker script form scratch
#
mk_linker: $(OBJDIR)/linker.lds

$(OBJDIR)/linker.lds: linker.lds.in
	$(CC) $(DEFINES) $(INCLUDES) -E -x c $< | grep -v "^\#" > $@	

###############################################################################
#
# This rule tells make how to build uart exapmle from *.o
#
$(OUTPUTFILE):$(COMMON_OBJ)
	$(LD) -o $@ $(LDFLAGS) $^

$(OBJDIR)/%.o: %.c
	@echo [CC] $<
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) -o $@ $<
	
$(OBJDIR)/%.o: %.S
	@echo [CC] $<
	$(CC) -c $(INCLUDES) $(CFLAGS) -o $@ $<

###############################################################################
#
#
.PHONY: all
all: mk_linker $(OUTPUTFILE)

###############################################################################
#
#
.PHONY: clean 
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(OBJDIR)/*.lds
	rm -f $(OUTPUTFILE)

###############################################################################
#
# EOF
#	
	