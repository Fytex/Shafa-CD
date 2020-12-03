# Name of the project
PROJ_NAME=shafa

# .c files
C_SOURCE=$(wildcard ./src/*.c)

# .h files
H_SOURCE=$(wildcard ./src/*.h)

# Object files
OBJ=$(subst .c,.o,$(subst src,obj,$(C_SOURCE)))

# Compiler and linker
CC=gcc

# Flags for compiler
CC_FLAGS=-std=c17 \
         -c

#
# Compilation and linking
#

all: objFolder $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	@ echo 'Building binary using GCC linker: $@'
	$(CC) -o $@ $^
	@ echo 'Finished building binary: $@'
	@ echo ' '

./obj/%.o: ./src/%.c #./src/%.h
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '


./obj/shafa.o: ./src/shafa.c $(H_SOURCE)
	@ echo 'Building target using GCC compiler: $<'
	$(CC) $< $(CC_FLAGS) -o $@
	@ echo ' '

objFolder:
	@ mkdir -p obj

clean:
	@ rm -rf ./obj/*.o $(PROJ_NAME) *~
	@ rmdir obj

.PHONY: all clean