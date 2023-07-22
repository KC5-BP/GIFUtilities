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

#SRC=autopilot.c camera.c engine.c gyroscope.c motor.c navigation.c pwm.c
# Detail: Does not respect alphabetic order by default
SRC=$(wildcard *.c)

################
# Object files #
################

## Separated because of a lib. constraint on libdriver.a
#OBJ=$(SRC:.c=.o)

##########
# Ouputs #
##########

EXE=gifReader.elf

#########################################################################

.PHONY: clean

all: $(EXE)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(EXE)

