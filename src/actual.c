
// -------------------------------------------------------------------------- //

#define TRUE true
#define FALSE false

// Prints Output to .pbm Files
#define TO_FILE TRUE
// Run in Debug-Mode => Displays Neighbour-Count
// This Option only has an Effect if TO_FILE is false
#define DEBUG TRUE
#if TO_FILE == TRUE
    #undef DEBUG
    #define DEBUG FALSE
#endif

// Delay between rounds when the Game is displayed on the Terminal.
#define DELAY 0.5

// -------------------------------------------------------------------------- //

int game_of_life(int argc, char* argv[]);
void printUsage(const char* programName);
bool *** init (int width, int height, double density);
void uninit(bool *** cells, int height);
int print_cells_to_file(bool ** cells, int iStep, int width, int height);
void main_loop (bool *** cells, int width, int height, int steps);
void print_cells(bool ** cells, int width, int height);
void swap(bool *** a, bool *** b);

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
    bool *** cells = init(width, height, density);

    // Loop for the Amount specified in Steps
    main_loop(cells, width, height, steps);

    // Restore Terminal Output and show the cursor again
    printf("\x1B[?1049l\x1B[?25h");

    uninit(cells, height);

    return EXIT_SUCCESS;
}

// -------------------------------------------------------------------------- //

// Allocate Memory for 2 2d-Array Game Fields and initialize them
bool *** init (int width, int height, double density) {

    // Allocate Array of Pointers to the Arrays containing the Cells
    bool ** cell_arr1 = malloc (sizeof(bool *) * height);

    // Check that Malloc worked
    if (!cell_arr1) {
        printf("Malloc failed\n");
        exit(1);
    }

    bool ** cell_arr2 = malloc (sizeof(bool *) * height);

    // Check that Malloc worked
    if (!cell_arr2) {
        free(cell_arr1);
        printf("Malloc failed\n");
        exit(1);
    }

    bool *** cells = malloc(sizeof(bool **) * 2);
    cells[0] = cell_arr1;
    cells[1] = cell_arr2;

    // Check that Malloc worked
    if (!cells) {
        free(cell_arr1);
        free(cell_arr2);
        printf("Malloc failed\n");
        exit(1);
    }

    // Multiply the Density by the maximum number that rand() can return
    int i_density = RAND_MAX * density;

    // Allocate/Initialize the Arrays of Cells
    for (long iLauf = 0; iLauf < height; iLauf ++) {
        // Allocate the Fields side by side using twice the Width of a Board.
        cell_arr1[iLauf] = malloc(sizeof(bool) * width * 2);
        // Check that Malloc worked
        if (!cell_arr1[iLauf]) {
            printf("Malloc failed\n");
            // Free already allocated Memory
            for (; iLauf > 0; iLauf --) free(cell_arr1[iLauf-1]);
            free(cell_arr1);
            free(cell_arr2);
            free(cells);
            exit(1);
        }
        // Since the Fields are allocated next to each other, the second
        // Board simply points in the middle of the allocated Memory.
        cell_arr2[iLauf] = cell_arr1[iLauf] + width;
        // Initialize Cells
        for (int iLauf2 = 0; iLauf2 < (width * 2); iLauf2 ++) {
            if (rand() <= i_density) {
                cell_arr1[iLauf][iLauf2] = true;
            } else {
                cell_arr1[iLauf][iLauf2] = false;
            }
        }
    }

    return cells;
}

// -------------------------------------------------------------------------- //

// Free the Memory allocated by the Init Function
// NOTE: I could not find a better name for this funtion
void uninit(bool *** cells, int height) {
    // Determine which Field has the original Pointers
    // which were allocated.
    if (cells[0][0] < cells[1][0]) {
        for (long iLauf = 0; iLauf < height; iLauf ++)
            free(cells[0][iLauf]);
    } else {
        for (long iLauf = 0; iLauf < height; iLauf ++)
            free(cells[1][iLauf]);
    }
    free(cells[0]);
    free(cells[1]);
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
void main_loop (bool *** cells, int width, int height, int steps) {

    int neighbours;

    bool ** src = cells[0];
    bool ** dest = cells[1];

    for (int iStep = 0; iStep < steps; iStep ++) {
        // Display the Board (either in a File or on the Terminal)
        #if TO_FILE == TRUE
            if (print_cells_to_file(src, iStep, width, height) == -1) {
                // There was an Error with the File
                // I assume that following Tries will also fail, so I return.
                uninit(cells, height);
                return;
            }
        #else
            // Some Terminal ANSI-Commands to clear the Screen every Re-Render
            printf("\x1B[25l\x1B[3J\x1B[0;0H\x1B[34mRound %d:\n\n", iStep + 1);
            print_cells(dest, width, height);
        #endif
        // Calculate neighbours
        for (int iLauf = 0; iLauf < height; iLauf++) {
            for (int iLauf2 = 0; iLauf2 < width; iLauf2++) {
                // Reset Neighbour Count
                neighbours = 0;
                // Count all alive Neighbour Cells
                // I don't know how to write this more concise, because I have
                // to test for Field Boundaries
                if ((iLauf - 1) >= 0) {
                    neighbours += src[iLauf-1][iLauf2];
                    if ((iLauf2 - 1) >= 0)
                        neighbours += src[iLauf-1][iLauf2-1];
                    if ((iLauf2 + 1) < width)
                        neighbours += src[iLauf-1][iLauf2+1];
                }
                if ((iLauf2 - 1) >= 0)
                    neighbours += src[iLauf][iLauf2-1];
                if ((iLauf2 + 1) < width) {
                    neighbours += src[iLauf][iLauf2+1];
                }
                if ((iLauf + 1) < height) {
                    neighbours += src[iLauf+1][iLauf2];
                    if ((iLauf2 - 1) >= 0)
                        neighbours += src[iLauf+1][iLauf2-1];
                    if ((iLauf2 + 1) < width)
                        neighbours += src[iLauf+1][iLauf2+1];
                }
                // Check if Cell should be alive or dead.
                if (
                    ((src[iLauf][iLauf2]) && (neighbours == 2)) ||
                    (neighbours == 3)
                ) {
                    dest[iLauf][iLauf2] = true;
                } else {
                    dest[iLauf][iLauf2] = false;
                }
            }
        }
        #if DEBUG == TRUE
            getchar();
        #else
            sleep(DELAY);
        #endif
        // Swap Pointers, switching the Fields from the View of the CPU.
        swap(&src, &dest);
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

// Print the Cell Array to a .pbm-File named "gol_<iStep>.pbm" and
// write the contents of the Cell Array to it in the Plain PBM-Format
// See: http://netpbm.sourceforge.net/doc/pbm.html
int print_cells_to_file(bool ** cells, int iStep, int width, int height) {

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
    int file_name_len = 6 + 4 + get_digits(iStep) + 4 + 1;
    int image_header_len = 3 + get_digits(width) + 1 + get_digits(height);

    // Get the maximum Value of the Length ensuring that the other
    // Buffer will have enough space.
    if (file_name_len > (width + 1)) buf_len = file_name_len;
    if (image_header_len > buf_len)  buf_len = image_header_len;

    // Finally, reserve the actual Buffer
    char * buffer = malloc(buf_len);

    // Dynamically create the Filename and create a new File/truncate an
    // existing File
    snprintf(buffer, buf_len, "build/gol_%05d.pbm", iStep);
    FILE * fd = fopen(buffer, "w");

    // Check that the File was successfully opened.
    // Otherwise exit because opening the other Files will probably fail.
    // I cannot exit here because I cannot deallocate the Cells
    // because I have a Pointer to one of the Fields but don't know
    // if I have the right Pointers to free (fifty-fifty).
    if (!fd) {
        // Deallocate all allocated Memory and exit
        free(buffer);
        printf("Could not open Files!\nEnsure that the Files are not already \
                open in another File\n");
        return -1;
    };

    // Format the PBM Image Header : http://netpbm.sourceforge.net/doc/pbm.html
    // and write it into the File
    // => P4\n<Width> <Height>\n
    snprintf(buffer, buf_len, "P1\n%d %d\n", width, height);
    fwrite(buffer, sizeof(char), buf_len, fd);

    // Go through all Cells (Row by Row) and write a '0' (white) to the File
    // if it is alive and '1' (black) if it is dead.
    for (long iLauf = 0; iLauf < height; iLauf ++) {
        for (long iLauf2 = 0; iLauf2 < width; iLauf2 ++) {
            buffer[iLauf2] = cells[iLauf][iLauf2] ? '0' : '1';
        }
        buffer[width] = '\n';
        fwrite(buffer, sizeof(char), width + 1, fd);
    }

    // Free Heap allocated Objects
    free(buffer);
    fclose(fd);

    return 0;

}

// -------------------------------------------------------------------------- //

// Print the Cell Array to STDOUT using colors
void print_cells(bool ** cells, int width, int height) {

    for (long iLauf = 0; iLauf < height; iLauf ++) {
        for (long iLauf2 = 0; iLauf2 < width; iLauf2 ++) {
            // Print a green 'a' when the Cell is alive
            // Print a red 'd' when the Cell is dead
            printf(
                "\x1B[%dm %c",
                cells[iLauf][iLauf2] ? 32 : 31,
                cells[iLauf][iLauf2] ? 'a' : 'd'
            );
        }
        printf("\n");
    }
}

// -------------------------------------------------------------------------- //

// https://stackoverflow.com/questions/8403447/swapping-pointers-in-c-char-int#8403699
void swap(bool *** a, bool *** b) {
    bool ** temp = *a;
    *a = *b;
    *b = temp;
}

// -------------------------------------------------------------------------- //
