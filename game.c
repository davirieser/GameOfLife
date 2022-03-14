
// -------------------------------------------------------------------------- //

// Links:
// ------
// Sources ANSI Codes: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
//                     https://en.wikipedia.org/wiki/ANSI_escape_code#Description
// Source Gosper Gun : https://conwaylife.com/wiki/Gosper_glider_gun

// -------------------------------------------------------------------------- //

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

// -------------------------------------------------------------------------- //

#define EASY 0
#define ACTUAL 1
#define COMPLICATED 2

// Which Variant of the Program to use
// Variant 0 = Easy Variant => Uses a 2d-Cell Array and can display the
//                             Game on STDOUT or using .pbm Files.
//                             Game will be played on a Field with the Height
//                             and Width specified using the CMD-Line.
// Variant 1 = Actual Sol.  => This uses 2 2d-Arrays for storing the
//                             Game Board. At first I couldn't understand
//                             why the Assignment would ever want to create
//                             the Board twice, because I thought I still
//                             had to store the Cell-Struct but after some
//                             thought I realized that I only had to use bools.
//                             This one I documented rather sparsly beacause
//                             most of it is already documented in the
//                             Easy-Variant.
// Variant 2 = Complicated =>  Uses a 1-d Cell Array to keep track of
//                             Cells and can (theoretically) play the game into
//                             Infinity (in practive until an Integer Overflow
//                             occurs or Memory runs out).
#define VARIANT COMPLICATED

#define u8 uint8_t

// -------------------------------------------------------------------------- //

int game_of_life(int argc, char* argv[]);

// -------------------------------------------------------------------------- //

#if VARIANT == EASY
    #include "src/easy.c"
#elif VARIANT == ACTUAL
    #include "src/actual.c"
#elif VARIANT == COMPLICATED
    #include "src/complicated.c"
#else
    _Static_assert(true, "Please provide an acutal Variant!")
#endif

// -------------------------------------------------------------------------- //

int main (int argc, char* argv[]) {

    return game_of_life(argc, argv);

}

// -------------------------------------------------------------------------- //
