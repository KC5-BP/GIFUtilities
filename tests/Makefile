##########################################################################

#############
# Toolchain #
#############

# No toolchain => native

CC     =gcc
AS     =as
LD     =ld
AR     =ar
OBJCPY =objcopy

#########
# Flags #
#########

CFLAGS   =-Wall -g -lm
ASFLAGS  =-g
LDFLAGS  =-L. -ldriver -T
ARFLAGS  =-r
CPYFLAGS =-O binary

################
# Source files #
################

# Detail: Does not respect alphabetic order by default
#SRC=$(wildcard *.c)
SRC=$(wildcard *.c)
MODULES_DIR=../modules

STRUCT_SRC_DIR=$(MODULES_DIR)/gifStructure
STRUCT_SRC=$(STRUCT_SRC_DIR)/GIFGetStructure.c $(STRUCT_SRC_DIR)/GIFPrintStructure.c

CONTENT_SRC_DIR=$(MODULES_DIR)/gifContent
CONTENT_SRC=$(CONTENT_SRC_DIR)/GIFReadContent.c $(CONTENT_SRC_DIR)/GIFPrintContent.c

################
# Object files #
################

## Separated because of a lib. constraint on libdriver.a
#OBJ=$(SRC:.c=.o)

##########
# Ouputs #
##########

EXE_STRUCT=structGetter.elf
EXE_CONTENT=contentReader.elf

#########################################################################

.PHONY: clean

all: $(EXE_STRUCT) $(EXE_CONTENT)

$(EXE_STRUCT): main_GetStructure.c $(STRUCT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(EXE_CONTENT): main_ReadContent.c $(CONTENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *.elf

