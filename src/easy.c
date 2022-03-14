
// -------------------------------------------------------------------------- //

#include "cell.c"

// -------------------------------------------------------------------------- //

#define TRUE true
#define FALSE false

// Randomly fill the Grid with
#define RANDOM TRUE
// Create a Gosper Glider Gun
// Prerequesites: Height > 10 and Width > 35
// If the RANDOM Option is selected, the Glider Gun will likely be destroyed.
#define GOSPER_GUN FALSE

// Prints Output to .pbm Files
#define TO_FILE FALSE
// Run in Debug-Mode => Displays Neighbour-Count
// This Option only has an Effect if TO_FILE is false
#define DEBUG TRUE
#if TO_FILE == TRUE
    #undef DEBUG
    #define DEBUG FALSE
#endif
// Put .pbm Files into the build/-Directory
// The Folder already has to exist, otherwise an Error is returned.
// Only has an Effect if TO_FILE is set
#define TO_BUILD_DIR TRUE

#if (TO_FILE == TRUE) && (TO_BUILD_DIR == TRUE)
    #define BUILD_DIR "build/"
    #define FILE_FORMATTER BUILD_DIR "gol_%05d.pbm"
#else
    #define FILE_FORMATTER "gol_%05d.pbm"
#endif

// Delay between rounds when the Game is displayed on the Terminal.
#define DELAY 0.5

// Because of all the Options sometimes the Compiler would complain
// about unused Parameters which would be needed for other Options.
// This Macro is a No-OP but suppresses the Unused Warning.
#define UNUSED(x) (void)(x)

// -------------------------------------------------------------------------- //

struct Cell ** init (int width, int height, double density);
void main_loop (struct Cell ** cells, int width, int height, int steps);
void print_cells(struct Cell ** cells, int width, int height);
void create_gosper_gun(struct Cell ** cells, int x, int y, int width, int height);
void uninit(struct Cell ** cells, int height);
void print_cells_to_file(struct Cell ** cells, int iStep, int width, int height);

// -------------------------------------------------------------------------- //

void printUsage(const char* programName) {
    printf("usage: %s <width> <height> <density> <steps>\n", programName);
}

// -------------------------------------------------------------------------- //

int game_of_life(int argc, char* argv[]) {
    if(argc != 5) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const int width = atoi(argv[1]);
    const int height = atoi(argv[2]);
    const double density = atof(argv[3]);
    const int steps = atoi(argv[4]);

    // Seeding the random number generator so we get a different starting field
    // every time.
    srand(time(NULL));

    // Get temporary Screen, saving the current Terminal Output and hide the
    // Cursor
    printf("\x1B[?1049h\x1B[?25l");

    // Allocate a 2d-Array using Malloc
    struct Cell ** cells = init(width, height, density);

    // Loop for the Amount specified in Steps
    main_loop(cells, width, height, steps);

    // Restore Terminal Output and show the cursor again
    printf("\x1B[?1049l\x1B[?25h");

    uninit(cells, height);

    return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------- //

// Allocate Memory for the 2d-Array Game Field and initialize it
struct Cell ** init (int width, int height, double density) {

    // Allocate Array of Pointers to the Arrays containing the Cells
    struct Cell ** cells = malloc (sizeof(struct Cell *) * height);

    // Check that Malloc worked
    if (!cells) {
        printf("Malloc failed\n");
        exit(1);
    }

    #if RANDOM == TRUE
        // Multiply the Density by the maximum number that rand() can return
        int i_density = RAND_MAX * density;
    #else
        // Use the density-Parameter in some kind, otherwise the Compiler will
        // scream at me :(.
        UNUSED(density);
    #endif

    // Allocate/Initialize the Arrays of Cells
    for (long iLauf = 0; iLauf < height; iLauf ++) {
        cells[iLauf] = malloc(sizeof(struct Cell) * width);
        // Check that Malloc worked
        if (!cells[iLauf]) {
            printf("Malloc failed\n");
            // Free already allocated Memory
            for (; iLauf > 0; iLauf --) free(cells[iLauf-1]);
            free(cells);
            exit(1);
        }
        // Initialize Cells
        for (int iLauf2 = 0; iLauf2 < width; iLauf2 ++) {
            #if RANDOM == TRUE
                if (rand() <= i_density) {
                    cells[iLauf][iLauf2] = alive();
                } else {
                    cells[iLauf][iLauf2] = dead();
                }
            #else
                cells[iLauf][iLauf2] = dead();
            #endif
        }
    }


#if GOSPER_GUN == TRUE
    // Use the density-Parameter in some kind, otherwise the Compiler will
    // scream at me :(.
    create_gosper_gun(
        cells, density * width / 2, density * height / 2, width, height
    );
#endif

    return cells;
}

// Free the Memory allocated by the Init Function
// NOTE: I could not find a better name for this funtion
void uninit(struct Cell ** cells, int height) {
    for (long iLauf = 0; iLauf < height; iLauf ++) {
        free(cells[iLauf]);
    }
    free(cells);
}

// -------------------------------------------------------------------------- //

// Loop for the specified amount of Steps
//      1. Display the Board
//      2. Calculate the Sum of each Cells alive neighbours
//      3. Revive dead cells with 3 alive neighbours, kill cells
//         that do not have 2 or 3 neighbours and reset each Cells
//         Neighbour Count.
//      4. Short Delay
void main_loop (struct Cell ** cells, int width, int height, int steps) {

    for (int iStep = 0; iStep < steps; iStep ++) {
        // Display the Board (either in a File or on the Terminal)
        #if TO_FILE == TRUE
            print_cells_to_file(cells, iStep, width, height);
        #else
            // Some Terminal ANSI-Commands to clear the Screen every Re-Render
            printf("\x1B[25l\x1B[3J\x1B[0;0H\x1B[34mRound %d:\n\n", iStep + 1);
            print_cells(cells, width, height);
        #endif
        // Calculate neighbours
        for (int iLauf = 0; iLauf < height; iLauf++) {
            for (int iLauf2 = 0; iLauf2 < width; iLauf2++) {
                // Reset Neighbour Count
                cells[iLauf][iLauf2].neighbours = 0;
                // Count all alive Neighbour Cells
                // I don't know how to write this more concise, because I have
                // to test for Field Boundaries
                if ((iLauf - 1) >= 0) {
                    cells[iLauf][iLauf2].neighbours += cells[iLauf-1][iLauf2].alive;
                    if ((iLauf2 - 1) >= 0)
                        cells[iLauf][iLauf2].neighbours += cells[iLauf-1][iLauf2-1].alive;
                    if ((iLauf2 + 1) < width)
                        cells[iLauf][iLauf2].neighbours += cells[iLauf-1][iLauf2+1].alive;
                }
                if ((iLauf2 - 1) >= 0)
                    cells[iLauf][iLauf2].neighbours += cells[iLauf][iLauf2-1].alive;
                if ((iLauf2 + 1) < width) {
                    cells[iLauf][iLauf2].neighbours += cells[iLauf][iLauf2+1].alive;
                }
                if ((iLauf + 1) < height) {
                    cells[iLauf][iLauf2].neighbours += cells[iLauf+1][iLauf2].alive;
                    if ((iLauf2 - 1) >= 0)
                        cells[iLauf][iLauf2].neighbours += cells[iLauf+1][iLauf2-1].alive;
                    if ((iLauf2 + 1) < width)
                        cells[iLauf][iLauf2].neighbours += cells[iLauf+1][iLauf2+1].alive;
                }

            }
        }
        // Revive previously Cells, remove dead Cells and reset neighbour-Count
        for (int iLauf = 0; iLauf < height; iLauf++) {
            for (int iLauf2 = 0; iLauf2 < width; iLauf2++) {
                // A Cell is alive if it is already alive and has 2 alive
                // neighbours or if it has 3 alive neighbours (ignoring
                // if it is alive or dead)
                if (
                    (
                        (cells[iLauf][iLauf2].alive) &&
                        (cells[iLauf][iLauf2].neighbours == 2)
                    ) ||
                    (cells[iLauf][iLauf2].neighbours == 3)
                ) {
                    cells[iLauf][iLauf2].alive = true;
                } else {
                    cells[iLauf][iLauf2].alive = false;
                }
            }
        }
        #if DEBUG == TRUE
            getchar();
        #else
            sleep(DELAY);
        #endif
    }

}

// -------------------------------------------------------------------------- //

// Print the Cell Array to STDOUT using colors
void print_cells(struct Cell ** cells, int width, int height) {

    for (long iLauf = 0; iLauf < height; iLauf ++) {
        for (long iLauf2 = 0; iLauf2 < width; iLauf2 ++) {
            #if DEBUG == FALSE
                // Print a green 'a' when the Cell is alive
                // Print a red 'd' when the Cell is dead
                printf(
                    "\x1B[%dm %c",
                    cells[iLauf][iLauf2].alive ? 32 : 31,
                    cells[iLauf][iLauf2].alive ? 'a' : 'd'
                );
            #else
                // Print the number of neighbours of the Cell in the last
                // Generation in green or red depending on whether it
                // is alive in this Generation.
                printf(
                    "\x1B[%dm %d",
                    cells[iLauf][iLauf2].alive ? 32 : 31,
                    cells[iLauf][iLauf2].neighbours
                );
            #endif
        }
        printf("\n");
    }
}

// -------------------------------------------------------------------------- //

// Count how many digits the number n has
int get_digits (int n) {
    int count = 0;
    while (n > 0) {
        count ++;
        n /= 10;
    }
    return count;
}

// -------------------------------------------------------------------------- //

// Print the Cell Array to a .pbm-File named "gol_<iStep>.pbm" and
// write the contents of the Cell Array to it in the Plain PBM-Format
// See: http://netpbm.sourceforge.net/doc/pbm.html
void print_cells_to_file(struct Cell ** cells, int iStep, int width, int height) {

    // Calculate the maximum number of Bits the Buffer has to have for it to
    // be used for formatting the Cells, storing the Filename and
    // the Image Header.
    // Image Header = 3 (p1\n) + Digits of Width (%d) + 1 + Digits of Height (%d)
    // File name = 4 (gol) + Digits of Step Counter (%d) + 4 (.pbm) + 1 (\0)
    // Cell Buffer Width = width + 1 ()
    // NOTE: I wrote this before realising that I should pad the File Number
    //       with 5 zeroes but the principle still stands considering there
    //       is a chance that someone will render more than 100000 Steps.
    int buf_len = width + 1;
    int file_name_len = sizeof(FILE_FORMATTER) + get_digits(iStep) + 1;
    int image_header_len = 3 + get_digits(width) + 1 + get_digits(height);

    // Get the maximum Value of the Length ensuring that the other
    // Buffer will have enough space.
    if (file_name_len > (width + 1)) buf_len = file_name_len;
    if (image_header_len > buf_len)  buf_len = image_header_len;

    // Finally, reserve the actual Buffer
    char * buffer = malloc(buf_len);

    // Format the Filename and create the new File/truncate an
    // existing File
    snprintf(buffer, buf_len, FILE_FORMATTER, iStep);
    FILE * fd = fopen(buffer, "w");

    // Check that the File was successfully opened.
    // Otherwise exit because opening the other Files will probably fail.
    if (!fd) {
        // Deallocate all allocated Memory and exit
        free(buffer);
        uninit(cells, height);
        printf("Could not open Files!\nEnsure that the Files are not already \
                open in another File\n");
        exit(1);
    };

    // Format the PBM Image Header : http://netpbm.sourceforge.net/doc/pbm.html
    // and write it into the File
    // => P4\n<Width> <Height>\n
    int bytes = snprintf(buffer, buf_len, "P1\n%d %d\n", width, height);
    fwrite(buffer, sizeof(char), bytes, fd);

    // Go through all Cells (Row by Row) and write a '0' (white) to the File
    // if it is alive and '1' (black) if it is dead.
    long iLauf2;
    for (long iLauf = 0; iLauf < height; iLauf ++) {
        for (iLauf2 = 0; iLauf2 < width; iLauf2 ++) {
            buffer[iLauf2] = cells[iLauf][iLauf2].alive ? '0' : '1';
        }
        // On the last run through iLauf2 should be equal to width
        // and since at least width + 1 is allocated this is fine
        buffer[iLauf2] = '\n';
        fwrite(buffer, sizeof(char), iLauf2 + 1, fd);
    }

    // Free Heap allocated Objects
    free(buffer);
    fclose(fd);

}

// -------------------------------------------------------------------------- //

// Create a Gosper Gun at the Location specified by x and y in the Cell Array
void create_gosper_gun(
    struct Cell ** cells, int x, int y, int width, int height
) {

    if ((width < (36 + x)) || (height < (10 + y))) {
        uninit(cells, height);
        printf("Could not create Gosper Gun!\nPlease create a bigger Grid!\n");
        exit(1);
    }

    cells[4+y][0+x].alive = true;
    cells[5+y][0+x].alive = true;
    cells[4+y][1+x].alive = true;
    cells[5+y][1+x].alive = true;

    cells[4+y][10+x].alive = true;
    cells[5+y][10+x].alive = true;
    cells[6+y][10+x].alive = true;

    cells[3+y][11+x].alive = true;
    cells[7+y][11+x].alive = true;

    cells[2+y][12+x].alive = true;
    cells[8+y][12+x].alive = true;

    cells[2+y][13+x].alive = true;
    cells[8+y][13+x].alive = true;

    cells[5+y][14+x].alive = true;

    cells[3+y][15+x].alive = true;
    cells[7+y][15+x].alive = true;

    cells[4+y][16+x].alive = true;
    cells[5+y][16+x].alive = true;
    cells[6+y][16+x].alive = true;

    cells[5+y][17+x].alive = true;

    cells[2+y][20+x].alive = true;
    cells[3+y][20+x].alive = true;
    cells[4+y][20+x].alive = true;

    cells[2+y][21+x].alive = true;
    cells[3+y][21+x].alive = true;
    cells[4+y][21+x].alive = true;

    cells[1+y][22+x].alive = true;
    cells[5+y][22+x].alive = true;

    cells[0+y][24+x].alive = true;
    cells[1+y][24+x].alive = true;
    cells[5+y][24+x].alive = true;
    cells[6+y][24+x].alive = true;

    cells[2+y][34+x].alive = true;
    cells[3+y][34+x].alive = true;
    cells[2+y][35+x].alive = true;
    cells[3+y][35+x].alive = true;

}

// -------------------------------------------------------------------------- //
