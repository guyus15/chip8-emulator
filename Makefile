# OBJS specifies which files to compile as part of the project
OBJS = main.c chip8.c

# OBJ_NAME specifies the name of our executable
OBJ_NAME = main

# This is the target that compiles our executable
main : $(OBJS)
	gcc $(OBJS) -o $(OBJ_NAME) -lSDL2 -g

# --- Testing ---

#TEST_OBJS specifies which files to compile as part of the test project
TEST_OBJS = chip8.c chip8_test.c

# TEST_OBJ_NAME specifies the name of our test executable
TEST_OBJ_NAME = chip8_test

# This is the target that compiles our test executable
test: $(TEST_OBJS)
	gcc $(TEST_OBJS) -o $(TEST_OBJ_NAME) -lSDL2 -g
