// -------------------------------------------------------------------------- //

#include "cell.c"

// -------------------------------------------------------------------------- //

#define CHUNK_SIZE 5
#define INNER_STRUCT struct Cell
#define CHUNK_POINTER_FIELD x

#include "cell_alloc.c"

// -------------------------------------------------------------------------- //

#if DEBUG == TRUE
    #define OUTPUT_ITERATOR TRUE
    #define OUTPUT_CREATE_TEMP FALSE
    #define OUTPUT_REVIVE_CELLS TRUE
    #define OUTPUT_COMPARE TRUE
    #define OUTPUT_NEIGHBOURS TRUE
#endif

// -------------------------------------------------------------------------- //

// Because of all the Options sometimes the Compiler would complain
// about unused Parameters which would be needed for other Options.
// This Macro is a No-OP but suppresses the Unused Warning.
#define UNUSED(x) (void)(x)

#define RED "\x1B[31m"
#define BLUE "\x1B[34m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define GRAY "\x1B[90m"
#define MAGENTA "\x1B[35m"
#define DEFAULT "\x1B[0m"

#define BG_RED "\x1B[41m"
#define BG_GREEN "\x1B[42m"
#define BG_BLUE "\x1B[44m"

#define UNDERLINE "\x1B[4m"
#define NO_UNDERLINE "\x1B[24m"
#define BOLD "\x1B[1m"

// -------------------------------------------------------------------------- //

#define SET_REG(x, num) (x = num)
#define CLEAR_REG(x) (x = 0)

// https://aticleworld.com/macros-for-bit-manipulation-c-cpp/
#define SET_BIT(x, pos) (x |= (1U << pos))
#define SET_BITS(x, num) (x |= num)

#define CLEAR_BIT(x, pos) (x &= (~(1U << pos)))
#define CLEAR_BITS(x, num) (x &= (~num))

#define TOGGLE_BIT(x, pos) (x ^= (1U << pos))
#define TOGGLE_BITS(x, num) (x ^= num)

#define CHECK_BIT(x, pos) ((x & (1U << pos)) >> pos)

// -------------------------------------------------------------------------- //

int is_neighbour (struct Cell self, struct Cell other);
void main_loop (const int steps);
int count_set_bits(struct Cell cell);
void direction_test ();
int compare_cells (struct Cell * self, struct Cell * other);
void change_pos (long * x, long * y, u8 direction);
void create_temp_cells (struct Cell * self, u8 directions);

// -------------------------------------------------------------------------- //

// Since this is not multithreaded, I will declare these as global, so I
// don't need to pass them down through each function
struct MemoryManager alive_cells = {
    .chunks = 0 ,
    .num_elem = 0,
    .allocated_chunks = 0
};

struct MemoryManager temp_cells = {
    .chunks = 0,
    .num_elem = 0,
    .allocated_chunks = 0
};

// -------------------------------------------------------------------------- //

// Create a Direction Namespace/Enum for storing Direction Constants
const struct {
    const u8 UP;
    const u8 DOWN;
    const u8 RIGHT;
    const u8 LEFT;
    const u8 UP_RIGHT;
    const u8 UP_LEFT;
    const u8 DOWN_RIGHT;
    const u8 DOWN_LEFT;
} DIRECTION = {
    .UP = 1 << 0,
    .DOWN = 1 << 1,
    .RIGHT = 1 << 2,
    .LEFT = 1 << 3,
    .UP_RIGHT = 1 << 4,
    .UP_LEFT = 1 << 5,
    .DOWN_RIGHT = 1 << 6,
    .DOWN_LEFT = 1 << 7
};

// -------------------------------------------------------------------------- //

// Reverse a Direction
// Return 0 if no valid Direction was passed
u8 reverse_direction (u8 direction) {
    if (direction == DIRECTION.UP)
        return DIRECTION.DOWN;
    else if (direction == DIRECTION.DOWN)
        return DIRECTION.UP;
    else if (direction == DIRECTION.RIGHT)
        return DIRECTION.LEFT;
    else if (direction == DIRECTION.LEFT)
        return DIRECTION.RIGHT;
    else if (direction == DIRECTION.UP_RIGHT)
        return DIRECTION.DOWN_LEFT;
    else if (direction == DIRECTION.UP_LEFT)
        return DIRECTION.DOWN_RIGHT;
    else if (direction == DIRECTION.DOWN_RIGHT)
        return DIRECTION.UP_LEFT;
    else if (direction == DIRECTION.DOWN_LEFT)
        return DIRECTION.UP_RIGHT;
    return 0;
}

// For Debugging Purposes
const char * direction_to_string (u8 direction) {
    if (direction == DIRECTION.UP)
        return "UP";
    else if (direction == DIRECTION.DOWN)
        return "DOWN";
    else if (direction == DIRECTION.RIGHT)
        return "RIGHT";
    else if (direction == DIRECTION.LEFT)
        return "LEFT";
    else if (direction == DIRECTION.UP_RIGHT)
        return "UP RIGHT";
    else if (direction == DIRECTION.UP_LEFT)
        return "UP LEFT";
    else if (direction == DIRECTION.DOWN_RIGHT)
        return "DOWN RIGHT";
    else if (direction == DIRECTION.DOWN_LEFT)
        return "DOWN LEFT";
    return "Invalid Direction";
}

// -------------------------------------------------------------------------- //

// TEST is set/defined via the Command Line using <gcc complicated.c -DTEST>
#ifndef TEST

    int game_of_life(int argc, char* argv[]) {

        UNUSED(argc);
        UNUSED(argv);

        // atoi returns 0 if it could not convert the number.
        const int steps = atoi(argv[4]);

        // Check if the Steps could be converted and are positive
        if (steps <= 0) {
            printf("Please input a valid number of Steps bigger than 0\n");
            exit(1);
        }

        if (init_chunks(&alive_cells) < 0) exit(1);
        if (init_chunks(&temp_cells) < 0) {
            deallocate_chunks(&alive_cells);
            exit(1);
        };

        add_elem(&alive_cells, new_cell(1, 1));
        add_elem(&alive_cells, new_cell(1, 2));
        add_elem(&alive_cells, new_cell(1, 3));

        main_loop(steps);

        deallocate_chunks(&alive_cells);
        deallocate_chunks(&temp_cells);

        return 0;

    }

#else

    int game_of_life(int argc, char* argv[]) {

        UNUSED(argc);
        UNUSED(argv);

        direction_test();

        if (init_chunks(&alive_cells) < 0) exit(1);
        if (init_chunks(&temp_cells) < 0) {
            deallocate_chunks(&alive_cells);
            exit(1);
        };

        for (int i = 0; i < 21; i++) {
            add_elem(&alive_cells, new_cell(i, i * 2));
        }

        struct MemoryIterator i = Iter.iter(&alive_cells);
        struct Cell * p = Iter.peek(&i);
        struct Cell * c = Iter.next(&i);

        while (c != NULL) {
            printf("\t\tPeeking: %p (%ld, %ld)\n", p, p->y, p->x);
            p = Iter.peek(&i);
            printf("\tNext Cell: %p (%ld, %ld)\n", c, c->y, c->x);
            c = Iter.next(&i);
        }

        printf(RED "Removing Cells\n");

        for (int i = 0; i < 11; i++) {
            remove_elem(&alive_cells, 0);
        }

        c = Iter.next(&i);

        while (c != NULL) {
            printf("\tNext Cell: %p (%ld, %ld)\n", c, c->y, c->x);
            c = Iter.next(&i);
        }

        deallocate_chunks(&alive_cells);
        deallocate_chunks(&temp_cells);

        return 0;

    }

// -------------------------------------------------------------------------- //

    void direction_test () {
        u8 directions[8] = {
            DIRECTION.UP, DIRECTION.DOWN, DIRECTION.RIGHT,
            DIRECTION.LEFT, DIRECTION.UP_RIGHT, DIRECTION.UP_LEFT,
            DIRECTION.DOWN_RIGHT, DIRECTION.DOWN_LEFT
        };
        u8 reverse;
        long initial_x = 5, initial_y = 3, x = initial_x, y = initial_y;

        for (u8 i = 0; i < 8; i ++) {
            reverse = reverse_direction(directions[i]);
            printf("Direction %s: %i => %s: %i\n",
                direction_to_string(directions[i]), directions[i],
                direction_to_string(reverse), reverse
            );
        }

        printf("\n");

        for (u8 i = 0; i < 8; i ++) {
            change_pos(&x, &y, directions[i]);
            printf("Initial (%ld, %ld) => %s => (%ld, %ld)\n",
                initial_x, initial_y,
                direction_to_string(directions[i]),
                x, y
            );
            x = initial_x;
            y = initial_y;
        }
    }

#endif

// -------------------------------------------------------------------------- //

void main_loop (const int steps) {

    // Variable Declarations

    struct MemoryIterator curr_iter, alive_iterator;
    // NOTE: Why tf do I need the second "*"?
    //       Why does it not assume that both are Pointers?
    struct Cell * curr_cell, * cmp_cell;

    // Keep Track of the current Cells neighbours
    u8 curr_neighbours;
    // Helper Variable for storing Directions.
    int direction;
    // Helper Variable for counting Direction Bits.
    int num_bits;

    // Keep Track of the number of rounds already elapsed.
    int step_counter = 0;

// -------------------------------------------------------------------------- //

    // Keep looping until no more Cells are alive or until the Step Limit is reached
    while ((alive_cells.num_elem > 0) && (step_counter < steps)) {
        // Setup Memory Iterators
        alive_iterator = Iter.iter(&alive_cells);
        curr_cell = Iter.next(&alive_iterator);
        // Start new Round
        step_counter ++;
        printf(RED "\nRound %d\n" DEFAULT, step_counter);

        // TODO: Implement Graphic Interface
        #if DEBUG == FALSE
            while (curr_cell != NULL) {
                printf(GREEN "\tCell %ld (%ld, %ld)\n", alive_iterator.curr_idx, curr_cell->y, curr_cell->x);
                curr_cell = Iter.next(&alive_iterator);
            }
            // Reset Alive Cell Iterator
            alive_iterator = Iter.iter(&alive_cells);
            curr_cell = Iter.next(&alive_iterator);
        #endif

// -------------------------------------------------------------------------- //

        // Calculate Neighbours for all alive Cells
        while (curr_cell != NULL) {
            // Reset Alive Cell Iterator
            curr_iter = alive_iterator;
            cmp_cell = Iter.next(&curr_iter);
            // Get the current Cells Neighbour Count (reset when checking if Cell is alive)
            curr_neighbours = curr_cell->neighbours;
            #if OUTPUT_ITERATOR == TRUE
                printf(GREEN "Current Cell (%ld): %p (%ld, %ld)\n", alive_iterator.curr_idx, curr_cell, curr_cell->y, curr_cell->x);
            #endif
            while (cmp_cell != NULL) {
                #if OUTPUT_COMPARE == TRUE
                    printf(YELLOW "\tCompare Cell: %p (%ld, %ld)\n", cmp_cell ,cmp_cell->y, cmp_cell->x);
                #endif
                // Compare Cells
                if ((direction = compare_cells(curr_cell, cmp_cell)) == -1) {
                    // Cells are neighbours so remove the later one.
                    // NOTE: This should not be able to happen so just print
                    //       an Error Message.
                    fprintf(stderr, BG_RED BLUE "There are two identical Cells at : (%ld, %ld)\n" DEFAULT, curr_cell->y, curr_cell->x);
                } else {
                    // Cells are Neighbours
                    // => Set the corresponding Direction Bit in the
                    //    neighbours-Field
                    SET_BITS(curr_cell->neighbours, direction);
                    SET_BITS(cmp_cell->neighbours, reverse_direction(direction));
                    // Add Direction to the current Cells Neighbour Variable
                    curr_neighbours |= direction;
                }
                cmp_cell = Iter.next(&curr_iter);
            }

// -------------------------------------------------------------------------- //

            // Loop through all Temporary Cells
            curr_iter = Iter.iter(&temp_cells);
            cmp_cell = Iter.next(&curr_iter);
            while (cmp_cell != NULL) {
                #if OUTPUT_COMPARE == TRUE
                    printf(YELLOW "\tCompare Cell: %p (%ld, %ld)\n", cmp_cell ,cmp_cell->y, cmp_cell->x);
                #endif
                // Compare Cells
                if ((direction = compare_cells(curr_cell, cmp_cell)) == -1) {
                    // Cells are neighbours so remove the later one.
                    // NOTE: This should not be able to happen so just print
                    //       an Error Message.
                    fprintf(stderr, BG_RED BLUE "There are two identical Cells at : (%ld, %ld)\n" DEFAULT, curr_cell->y, curr_cell->x);
                } else {
                    // Cells are Neighbours
                    // Because these Cells are not alive, only in consideration
                    // don't set the Direction Bit in the original (alive) Cell
                    // => Set the corresponding Direction Bit in the
                    //    neighbours-Field
                    SET_BITS(cmp_cell->neighbours, reverse_direction(direction));
                    // Add Direction to the current Cells Neighbour Variable
                    curr_neighbours |= direction;
                }
                cmp_cell = Iter.next(&curr_iter);
            }
            // Create Temporary Cells around the current Cell.
            create_temp_cells(curr_cell, curr_neighbours);
            // Get next Cell
            curr_cell = Iter.next(&alive_iterator);
        }

// -------------------------------------------------------------------------- //

        // Check which alive Cells stay alive
        curr_iter = Iter.iter(&alive_cells);
        curr_cell = Iter.next(&curr_iter);
        while (curr_cell != NULL) {
            num_bits = count_set_bits(*curr_cell);
            if ((num_bits == 2) || (num_bits == 3)) {
                // Reset Cells Neighbour Count
                curr_cell->neighbours = 0;
                // Get the next Cell
                curr_cell = Iter.next(&curr_iter);
                #if OUTPUT_REVIVE_CELLS
                    printf("\t\tSurviving Cell: (%ld, %ld) %d\n", curr_cell->y, curr_cell->x, num_bits);
                #endif
            } else {
                // Unalive the Cell
                remove_elem(&alive_cells, curr_iter.curr_idx - 1);
                // Get the Cell at the same place which replaced the old one
                curr_cell = Iter.previous(&curr_iter);
            }
        }

// -------------------------------------------------------------------------- //

        // Check which temporary Cells will resurrect
        curr_iter = Iter.iter(&temp_cells);
        curr_cell = Iter.next(&curr_iter);
        while (curr_cell != NULL) {
            num_bits = count_set_bits(*curr_cell);
            if (num_bits == 3) {
                // Reset Cells Neighbour Count
                curr_cell->neighbours = 0;
                add_elem(&alive_cells, *curr_cell);
                #if OUTPUT_REVIVE_CELLS
                    printf("\t\tRessurecting Cell (%ld, %ld) %d\n", curr_cell->y, curr_cell->x, num_bits);
                #endif
            }
            curr_cell = Iter.next(&curr_iter);
        }
        // Reset Temporary Cells
        // TODO: Implement a Function for removing all Elements because this
        // does not deallocate any chunks, which could become a Problem.
        temp_cells.num_elem = 0;

    }

}

// -------------------------------------------------------------------------- //

// Test if Cells are neighbours
// If they are neighbours, indicate as such in their Neighbour Fields
int compare_cells (struct Cell * self, struct Cell * other) {
    int neighbour = is_neighbour(*self, *other);
    switch (neighbour) {
        case 0:
            // Cells are not neighbours so do nothing
            return 0;
        case -1:
            // Cells are the same so remove the later one
            // NOTE: This should not be able to happen
            return -1;
        default:
            // Cells are neighbours
            #if OUTPUT_NEIGHBOURS == TRUE
                printf(BLUE "\t\tNeighbours: (%ld, %ld) (%ld, %ld) : %s => %s\n",
                    self->y, self->x, other->y, other->x,
                    direction_to_string(neighbour),
                    direction_to_string(reverse_direction(neighbour))
                );
            #endif
            return neighbour;
    }
}

// -------------------------------------------------------------------------- //

// Create all temporary Cells around the Cell self which didn't already exist
void create_temp_cells (struct Cell * self, u8 directions) {

    int bitmask = 1;
    u8 reverse;
    long x, y;

    while (bitmask <= 255) {

        if (!(directions & bitmask)) {

            x = self->x;
            y = self->y;

            reverse = reverse_direction(bitmask);

            change_pos(&x, &y, reverse);

            struct Cell c = new_cell(y, x);
            c.neighbours = reverse;

            add_elem(&temp_cells, c);

            #if OUTPUT_CREATE_TEMP == TRUE
                printf(GREEN "\t\t\tCreating Temp Cell at (%ld, %ld)\n", y, x);
            #endif

        }

        bitmask <<= 1;

    }

}

// -------------------------------------------------------------------------- //

// Change the Position according to the Direction facing
// (1, 1) => UP => (0, 1) => RIGHT => (0, 2) => DOWN_LEFT => (1, 1)
void change_pos (long * x, long * y, u8 direction) {
    if (direction == DIRECTION.UP)
        (*y) --;
    else if (direction == DIRECTION.DOWN)
        (*y) ++;
    else if (direction == DIRECTION.RIGHT)
        (*x) ++;
    else if (direction == DIRECTION.LEFT)
        (*x) --;
    else if (direction == DIRECTION.UP_RIGHT){
        (*x) ++;
        (*y) --;
    } else if (direction == DIRECTION.UP_LEFT) {
        (*x) --;
        (*y) --;
    } else if (direction == DIRECTION.DOWN_RIGHT) {
        (*x) ++;
        (*y) ++;
    } else if (direction == DIRECTION.DOWN_LEFT) {
        (*x) --;
        (*y) ++;
    }
}

// -------------------------------------------------------------------------- //

// Check if two Cells are neighbours
// Returns the Direction which is used to get from Cell self to
// Cell other if they are neighbours
// Returns 0 if they are not neighbours
// Returns -1 if they are the same Cell
int is_neighbour (struct Cell self, struct Cell other) {

    int x_dist = (self.x - other.x);
    int y_dist = (self.y - other.y);

    if (x_dist == 1) {
        if (y_dist == 1)
            return DIRECTION.DOWN_RIGHT;
        else if (y_dist == -1)
            return DIRECTION.UP_RIGHT;
        else if (y_dist == 0)
            return DIRECTION.RIGHT;
    } else if (x_dist == -1) {
        if (y_dist == 1)
            return DIRECTION.DOWN_LEFT;
        else if (y_dist == -1)
            return DIRECTION.UP_LEFT;
        else if (y_dist == 0)
            return DIRECTION.LEFT;
    } else if (x_dist == 0) {
        if (y_dist == 1)
            return DIRECTION.DOWN;
        else if (y_dist == -1)
            return DIRECTION.UP;
        else if (y_dist == 0)
            // Cells are the same
            return -1;
    }

    // Cells are not the same and not neighbours
    return 0;

}

// -------------------------------------------------------------------------- //

// https://www.tutorialspoint.com/c-cplusplus-program-to-count-set-bits-in-an-integer
int count_set_bits(struct Cell cell) {

    int count = 0;
    int n = cell.neighbours;

    while(n != 0) {
        if((n & 1) == 1)
            count++;
        n = n >> 1;
    }

    return count;

}

// -------------------------------------------------------------------------- //
