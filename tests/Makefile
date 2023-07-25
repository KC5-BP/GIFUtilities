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
SRC_FOLDER=../GIFUtilities
SRC_STRUCT=$(SRC_FOLDER)/GIFStructure/GIFGetStructure.c $(SRC_FOLDER)/GIFStructure/GIFPrintStructure.c

################
# Object files #
################

## Separated because of a lib. constraint on libdriver.a
#OBJ=$(SRC:.c=.o)

##########
# Ouputs #
##########

EXE_STRUCT=structGetter.elf

#########################################################################

.PHONY: clean

all: $(EXE_STRUCT)

$(EXE_STRUCT): main_GetStructure.c $(SRC_STRUCT)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(EXE_STRUCT)

