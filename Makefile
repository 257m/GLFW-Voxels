all: clean main

CC = clang
CFLAGS = -g -Wno-everything `pkg-config --cflags glfw3 glu gl` -I/nix/store/niq09gs0z4xpsn45n3bfy80s17819mnw-stb-20180211/include
LDFLAGS = `pkg-config --libs glfw3 glu gl` -lm

# This is the name of the executable we are going to create
EXE = main

# This is the default target that is going to be built when we run 'make'
all: clean $(EXE)

# This is the directory where our source will be stored
SRC_DIR = src

# This is the list of all source files
SRC = $(wildcard $(SRC_DIR)/*.c)

# This is the directory where our object files will be stored
OBJ_DIR = obj

# This is the list of object files that we need to create
OBJ = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRC))

# This is the rule to build the executable
$(EXE): $(OBJ)
		$(CC) $(LDFLAGS) $^ -o "$@"

# This is the rule to build the object files
$(OBJ_DIR)/%.o: src/%.c
		$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f main