# Name of the project
PROJ_NAME=shafa

# EXT .c file
SOURCE=$(shell find src/ -name '*.c')

#EXT .c file no pthread
SOURCE_NO_PTHREAD=$(shell find src/ -name '*.c' -not -name 'multithread.*')

# Object files
OBJ=$(subst .c,.o,$(subst src,obj,$(SOURCE)))

# Object files no pthread
OBJ_NO_PTHREAD=$(subst .c,.o,$(subst src,obj,$(SOURCE_NO_PTHREAD)))

# Compiler and linker
CC=gcc

# Flags for compiler
CC_FLAGS=-std=c17 \
         -c       \
         -MP      \
         -MD      \
	     -Wall

#
# Compilation and linking
#

seq: objFolder $(OBJ_NO_PTHREAD)
	@ echo 'Building binary using GCC linker: $(OBJ_NO_PTHREAD)'
	$(CC) -o $(PROJ_NAME) $(OBJ_NO_PTHREAD)
	@ echo 'Finished building binary - NO PTHREAD'
	@ echo ' '


pthread: objFolder $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker with -pthread: $@'
	$(CC) -o $@ $^ -pthread
	@ echo 'Finished building binary - PTHREAD'
	@ echo ' '

./obj/%.o: ./src/%.c
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '


objFolder:
	@ mkdir -p obj/modules/utils

clean:
	@ rm -rf $(PROJ_NAME) *~
	@ rm -rf obj
	@ echo 'Cleaned everything successfully'

.PHONY: all clean
