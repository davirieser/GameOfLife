
# ---------------------------------------------------------------------------- #

EXE_NAME=game

BUILD_DIR=build/

X=50
Y=20
DELAY=1
DENSITY=0.3
STEPS=100

CFLAGS=-std=c11 -Wall -Wextra -Werror -O -g -fsanitize=leak

# ---------------------------------------------------------------------------- #

all: run
	@ echo "Converting into .gif"
	@ convert -filter point -resize $(X)%$(Y)% -delay $(DELAY) $(BUILD_DIR)gol_*.pbm gol.gif

# ---------------------------------------------------------------------------- #

run: build
	@ # Ensure that the Build-Directory exists
	@ mkdir $(BUILD_DIR) -p
	@ echo "Running Exe\n./$(EXE_NAME)"
	@ ./$(EXE_NAME) $(X) $(Y) $(DENSITY) $(STEPS)

# ---------------------------------------------------------------------------- #

build: $(EXE_NAME)
	@ echo "Building Exe: $(EXE_NAME)"

test: CFLAGS+=-DTEST
test: $(EXE_NAME)
	@ ./$(EXE_NAME) $(X) $(Y) $(DENSITY) $(STEPS)

# ---------------------------------------------------------------------------- #

%: %.c
	@ $(CC) $(CFLAGS) -o $* $^

# ---------------------------------------------------------------------------- #

clean:
	@ $(RM) $(EXE_NAME) *.pbm *.gif
	@ $(RM) -rf $(BUILD_DIR)

# ---------------------------------------------------------------------------- #
