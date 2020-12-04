# Name of the project
PROJ_NAME=shafa

# Main .c file
C_MAIN=./src/$(PROJ_NAME).c

# EXT .c file
C_EXT=$(wildcard ./src/**/*.c)

# Object files
OBJ=$(subst .c,.o,$(subst src,obj,$(C_MAIN) $(C_EXT)))

# Compiler and linker
CC=gcc

# Flags for compiler
CC_FLAGS=-std=c17 \
         -c       \
		 -MP      \
		 -MD

#
# Compilation and linking
#

all: objFolder $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) -o $@ $^
	@ echo 'Finished building binary: $@'
	@ echo ' '


./obj/%.o: ./src/%.c
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '


objFolder:
	@ mkdir -p obj/modules

clean:
	@ rm -rf $(PROJ_NAME) *~
	@ rm -rf obj
	@ echo 'Cleaned everything successfully'

.PHONY: all clean
