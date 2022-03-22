
// -------------------------------------------------------------------------- //
// --- Explanation ---------------------------------------------------------- //
// -------------------------------------------------------------------------- //

// ---------------------    ---------------------    ---------------------
// |c1|c2|c3|...|cx|*ci| -> |c1|c2|c3|...|cx|*ci| -> |c1|c2|c3|...|cx|*ci|
// ---------------------    ---------------------    ---------------------
// cx  = Data Cell
// *ci = Chunk Pointer Cell => Stores a Pointer to the next Chunk in
//                             it's .x-Field.
//                             This is why the Chunk Pointer Index is one
//                             less than the CHUNK_SIZE.
//                             The last Chunk Pointer will be a NULL
//                             Pointer to indicate that no more chunks
//                             are allocated after it.
//
// NOTE: I think I have too many safeguards/redundant safeguards implemented,
//       but since this is not Rust, I thought better safe than sorry.

// Usage Manual:
//
// Your Code needs to define a few Values for this Code to work:
//
//      => (Optional) CHUNK_SIZE = Number of Elements per Chunk (default = 4096)
//      => INNER_STRUCT          = The Type
//      => CHUNK_POINTER_FIELD   = The Field of the Struct which holds the
//                                 Chunk Pointers
//                                 BEWARE: This Field needs to be able to
//                                 store a Pointer in it (long)

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

#define TRUE true
#define FALSE false

// Deallocate Chunks once they contain no more data.
#define DEALLOCATE_UNUSED_CHUNKS TRUE

// Debug Options
#ifndef DEBUG
    #define DEBUG FALSE
#endif
#if DEBUG == TRUE
    #ifndef OUTPUT_REDIRECTION
        #define OUTPUT_REDIRECTION FALSE
    #endif
    #ifndef OUTPUT_CELL_ASSIGN
        #define OUTPUT_CELL_ASSIGN FALSE
    #endif
    #ifndef OUTPUT_CELL_REMOVE
        #define OUTPUT_CELL_REMOVE FALSE
    #endif
    #ifndef OUTPUT_NEW_CHUNK
        #define OUTPUT_NEW_CHUNK   FALSE
    #endif
    #ifndef OUTPUT_FREE_CHUNKS
        #define OUTPUT_FREE_CHUNKS FALSE
    #endif
#endif

// -------------------------------------------------------------------------- //
// --- Memory Manager Options ----------------------------------------------- //
// -------------------------------------------------------------------------- //

// Weakly define the number of Elements per allocated Chunk.
// NOTE: Keep in mind that the number of actual Data Structs per Chunk is
//       CHUNK_SIZE - 1 because the CHUNK_POINTER takes up one slot.
#ifndef CHUNK_SIZE
    #define CHUNK_SIZE 4096
#endif

// If the CHUNK_SIZE is smaller than 1 the Cell-Array only consists of
// Chunk Pointers.
// At that Point you might as well use a Linked List.
// NOTE: Also probably throws an Error because it will loop trying to
//       allocate chunks in add_cell or allocate_chunk.
_Static_assert(
    CHUNK_SIZE > 1,
    "Please provide a Chunk Size that is bigger than 1"
);

// Calculate the Position at which the Pointer to the next Chunk is stored.
// NOTE: This has to be enclosed in Braces because otherwise
//       it might be wrongly replaced for Example:
//       0 % CHUNK_POINTER_IDX => becomes => 0 % CHUNK_SIZE - 1 = 0 - 1 = -1
#define CHUNK_POINTER_IDX (CHUNK_SIZE - 1)

// Create an Alias for the Data Element Type
typedef INNER_STRUCT Inner;

// -------------------------------------------------------------------------- //

struct MemoryManager {
    Inner * chunks;
    long num_elem;
    long allocated_chunks;
};

// -------------------------------------------------------------------------- //

// Helper Struct for iterating through Chunks in a MemoryManager
// Built according to the Rust-Iterator:
// https://doc.rust-lang.org/stable/std/iter/index.html
struct MemoryIterator {
    Inner * chunks;
    long * num_elem;
    long curr_idx;
    long curr_chunk;
};

// -------------------------------------------------------------------------- //

static struct MemoryIterator to_iter(struct MemoryManager * m) {
    struct MemoryIterator i = {
        .chunks = m->chunks,
        .num_elem = &m->num_elem,
        .curr_idx = 0,
        .curr_chunk = 0
    };
    return i;
}

// -------------------------------------------------------------------------- //

// Return the next Element as a Pointer.
// Returns NULL if all Elements where already returned.
// Resolves Chunk Pointers once it has reached the Chunk Pointer.
static Inner * next (struct MemoryIterator * i) {
    // Check that the Iterator is still in the allocated Memory.
    if (i->curr_idx < *i->num_elem) {
        // Check if the current Element is the Chunk Pointer
        if (((i->curr_idx + i->curr_chunk) % CHUNK_SIZE) == CHUNK_POINTER_IDX) {
            // Check that more chunks are allocated.
            if (((Inner *) i->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD) == NULL) {
                return NULL;
            }
            // Redirect to the next Chunk
            i->chunks = (Inner *) i->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD;
            i->curr_chunk++;
            #if OUTPUT_REDIRECTION == TRUE
                printf("Redirecting Chunk to %p\n", i->chunks);
            #endif
            return next(i);
        }
        return &i->chunks[(i->curr_idx++ + i->curr_chunk) % CHUNK_SIZE];
    }
    return NULL;
}

// -------------------------------------------------------------------------- //

// Return the next Element as a Pointer while leaving the Iterator unchanged.
// Returns NULL if all Elements where already returned.
// Resolves Chunk Pointers.
static Inner * peek (struct MemoryIterator * i) {
    // Check that the Iterator is still in the allocated Memory.
    if (i->curr_idx <= *(i->num_elem)) {
        // Check if the current Element is the Chunk Pointer
        if (((i->curr_idx + i->curr_chunk) % CHUNK_SIZE) == CHUNK_POINTER_IDX) {
            // Check that more chunks are allocated.
            if (((Inner *) i->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD) == NULL) {
                return NULL;
            }
            // Redirect to the next Chunk
            Inner * chunk = (Inner *) i->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD;
            #if OUTPUT_REDIRECTION == TRUE
                printf("Peeking Chunk %p\n", i->chunks);
            #endif
            return &chunk[0];
        }
        return &i->chunks[(i->curr_idx + i->curr_chunk) % CHUNK_SIZE];
    }
    return NULL;
}

// -------------------------------------------------------------------------- //

// TODO: This isn't tested yet but probably should work.
// Moves the Iterator backwards one Element and returns it.
// Because this is a single Linked List this only works until it is at the
// start of the Chunk. Once it is at the start of the Chunk it returns NULL.
static Inner * next_back (struct MemoryIterator * i) {
    // Check that the Iterator is still in the allocated Memory.
    if ((i->curr_idx - 1) < *(i->num_elem)) {
        // Check if the Index is at the start of the Chunk.
        if (((i->curr_idx + i->curr_chunk) % CHUNK_SIZE) == 0)
            return NULL;
        return &i->chunks[(i->curr_idx-- + i->curr_chunk) % CHUNK_SIZE];
    }
    return NULL;
}

// -------------------------------------------------------------------------- //

// Return the previous Element as a Pointer while leaving the Iterator unchanged.
// This works because next does not immidiately resolve Chunk Pointers.
// Returns NULL if all Elements where already returned.
// Resolves Chunk Pointers and leaves the Iterator unchanged.
static Inner * previous (struct MemoryIterator * i) {
    // Check that the Iterator is still in the allocated Memory.
    if ((i->curr_idx - 1) < *(i->num_elem)) {
        // Check if the Index is at the start of the Chunk.
        if (((i->curr_idx + i->curr_chunk) % CHUNK_SIZE) == 0)
            return NULL;
        return &i->chunks[((i->curr_idx - 1) + i->curr_chunk) % CHUNK_SIZE];
    }
    return NULL;
}

// -------------------------------------------------------------------------- //

// TODO: I am too dumb for this rn
// Skip the specified Amount of Elements.
// Resolves Chunk Pointers.
// static void skip (struct MemoryIterator * i, int elems) {
//     if (elems < 1) return;
//     int target = i->curr_idx + elems;
//     if (target < *(i->num_elem)) {
//         // If target is outside the current Chunk, move to next one
//         if (target > CHUNK_SIZE) {
//             // Redirect to the next Chunk
//             i->chunks = (Inner *) i->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD;
//             i->curr_chunk++;
//         }
//
//     }
// }

// -------------------------------------------------------------------------- //

static struct MemoryIterator clone_iter (struct MemoryIterator * i) {
    struct MemoryIterator clone = {
        .chunks = i->chunks,
        .num_elem = i->num_elem,
        .curr_idx = i->curr_idx,
        .curr_chunk = i->curr_chunk
    };
    return clone;
}

// -------------------------------------------------------------------------- //

// Call a Closure on each Iterator Element.
static void for_each (struct MemoryIterator * i, void (*func)(Inner *)) {
    Inner * inner = next(i);
    while (inner != NULL) {
        func(inner);
        inner = next(i);
    }
}

// -------------------------------------------------------------------------- //

//
struct {
    struct MemoryIterator (*iter)(struct MemoryManager *);
    Inner * (*next) (struct MemoryIterator *);
    Inner * (*peek) (struct MemoryIterator *);
    Inner * (*next_back) (struct MemoryIterator *);
    struct MemoryIterator (*clone_iter) (struct MemoryIterator * i);
    Inner * (*previous) (struct MemoryIterator * i);
    void (*for_each) (struct MemoryIterator * i, void (*func)(Inner *));
} Iter = {
    .iter = to_iter,
    .next = next,
    .peek = peek,
    .next_back = next_back,
    .clone_iter = clone_iter,
    .previous = previous,
    .for_each = for_each
};

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

// NOTE: These Prototypes have to be declared beneath the MemoryManager Struct
// because otherwise the Compiler will complain that MemoryManager is
// redefined.
// This is the first time I have seen that Interaction.
int init_chunks (struct MemoryManager * m);
int allocate_chunk(struct MemoryManager * m);
static int deallocate_last_chunk (struct MemoryManager * m);
void deallocate_chunks (struct MemoryManager * m);
static void deallocate_chunks_inner (Inner * c);
int add_elem (struct MemoryManager * m, Inner c);
int remove_elem (struct MemoryManager * m, long idx);
Inner * get_elem (struct MemoryManager m, long idx);
static Inner * get_elem_inner (Inner * c, long idx);
Inner * get_chunk_pointer (struct MemoryManager m, long idx);
static Inner * get_chunk_pointer_inner (Inner * c, long idx);
void print_chunks(struct MemoryManager m, char * (*display)(Inner));
void print_chunk_pointers(struct MemoryManager m);

// -------------------------------------------------------------------------- //

// Allocate initial Memory Chunks
int init_chunks (struct MemoryManager * m) {

    // Allocate Memory for the alive Cells.
    void * chunk = malloc (CHUNK_SIZE * sizeof(Inner));
    // Check that the memory allocation worked.
    if (chunk == NULL) return -1;

    // Assign the new Chunk to the Alive-Cells and make sure that
    // the Chunk Pointers is NULL indicating that no Chunks are
    // allocated after it.
    m->chunks = chunk;
    m->chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD = 0;
    m->allocated_chunks ++;
    // Reset Element Count
    m->num_elem = 0;

    #if OUTPUT_NEW_CHUNK
        printf("Initial Chunk: %p\n", chunk);
    #endif

    return 0;
}

// -------------------------------------------------------------------------- //

int allocate_chunk(struct MemoryManager * m) {
    // Allocate new Chunk
    Inner * new_chunk = malloc(CHUNK_SIZE * sizeof(Inner));
    // Check that the Memory has been allocated.
    if (new_chunk == NULL) return -1;
    // Get the Last Chunk Pointer
    Inner * last_chunk_pointer = get_chunk_pointer(*m, -1);
    if (last_chunk_pointer == NULL) {
        return -1;
    }
    // Create Pointer to the new Chunk at the End of the last Chunk.
    last_chunk_pointer->x = (long) new_chunk;
    #if OUTPUT_NEW_CHUNK == TRUE
        printf("New Chunk: %p\n", (Inner *) last_chunk_pointer->x);
    #endif
    // Create null pointer instead of pointer to the next Chunk.
    // This is basically a sign that no more chunks are allocated after this.
    new_chunk[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD = 0;
    // Increment number of allocated Chunks
    m->allocated_chunks += 1;
    return 0;
}

// -------------------------------------------------------------------------- //

// Private Helper Funtion for remove_cell
static int deallocate_last_chunk (struct MemoryManager * m) {
    // Get the last Chunk Pointer which shouldn't contain
    // any more Data.
    Inner * p1 = get_chunk_pointer(*m, m->allocated_chunks - 1);

    // Check that the Pointer is not NULL.
    if (p1 == NULL) return -1;

    #if OUTPUT_FREE_CHUNKS == TRUE
        printf("Freeing: %p\n", (Inner *) p1->x);
    #endif

    // Deallocate the Chunk AFTER checking for Null Pointers.
    free((Inner *) p1->x);
    p1->x = (long) NULL;
    m->allocated_chunks--;

    return 0;
}

// -------------------------------------------------------------------------- //

// Free all Allocated Memory of a MemoryManager
void deallocate_chunks (struct MemoryManager * m) {
    deallocate_chunks_inner(m->chunks);
}

// Helper Function for deallocate_cells
// This takes a reference to a Cell-Array and recursively frees all
// chunks which are allocated in it.
static void deallocate_chunks_inner (Inner * c) {
    // Check if the passed Pointer is the last Chunk Pointer
    // If a chunk is allocated after this skip the If-Block.
    if ((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD == NULL) {
        #if OUTPUT_FREE_CHUNKS == TRUE
            printf("Freeing: %p\n", c);
        #endif
        // Then free it and return.
        // At this Point the Chunk Pointers will recursively be freed.
        free(c);
        c = NULL;
        return;
    }
    // Call this recursively passing in the next Chunk.
    deallocate_chunks_inner((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD);
    #if OUTPUT_FREE_CHUNKS == TRUE
        printf("Freeing: %p\n", c);
    #endif
    // Free the current Chunk.
    // At this point this is the last allocated Chunk because all
    // Chunks before it were recursively freed.
    free(c);
    c = NULL;
    return;
}

// -------------------------------------------------------------------------- //

// Add Cell, allocating more Space if needed
// Returns  0 if Operation succeded.
// Returns -1 if no Space could be allocated anymore
int add_elem (struct MemoryManager * m, Inner c) {
    // This needs to use get_elem_inner because it has to bypass the
    // Index Check in get_elem.
    // The Index Check in get_elem is supposed to catch Users trying to
    // get a reference to an unitialised Element but in add_elem an
    // unitialised Element is overwritten.
    Inner * p = get_elem_inner(m->chunks, m->num_elem);
    // Check that the still is space available in the current Chunk.
    if (p == NULL) {
        // If not allocate another Chunk
        if (allocate_chunk(m) >= 0)
            return add_elem(m, c);
    } else {
        #if OUTPUT_CELL_ASSIGN == TRUE
            printf("\tAdding Cell: %p\n", p);
        #endif
        // If there still is space append the Data Element into the Chunk.
        *p = c;
        m->num_elem ++;
        return 0;
    }
    // If allocating more space failed, return an Error Code.
    return -1;
}

// -------------------------------------------------------------------------- //

// Overwrite the Cell at the specified Index
// with the Cell at the last Position, removing it.
// This is possible because the order of the cells
// does not matter.
int remove_elem (struct MemoryManager * m, long idx) {
    // Check that the Cell which is trying to be removed is actually
    // allocated.
    if ((idx < 0) || (idx >= m->num_elem)) return -1;
    // If the Cell trying to be removed is the last Cell
    // just decrease the Cell-Count (the next add_cell-Call
    // will overwrite it).
    else if (idx == (m->num_elem - 1)) {
    #if OUTPUT_CELL_REMOVE == TRUE
        printf(
            "Removing last Cell %ld = %p\n",
            idx, get_elem(*m, m->num_elem - 1)
        );
    #endif
        m->num_elem--;
        // If no more Cells remain in the last Chunk deallocate it.
        if ((m->num_elem % CHUNK_POINTER_IDX) == (CHUNK_POINTER_IDX - 1)) {
            deallocate_last_chunk(m);
        }
        return 0;
    }
    // Get the Cell which should be replaced and the last Cell
    Inner * p1 = get_elem(*m, m->num_elem - 1);
    Inner * p2 = get_elem(*m, idx);
    // Check that both Cells exist.
    if ((p1 == NULL) || (p2 == NULL)) {
        return -1;
    }
    #if OUTPUT_CELL_REMOVE == TRUE
        printf(
            "Removing Cell %ld = %p (replacing with: %ld = %p)\n",
            idx, p2, m->num_elem-1, p1
        );
    #endif
    // Overwrite the Cell at the specified Index with the
    // Cell at the End of the Array.
    // Because the order of the cells doesn't matter this can be done.
    *p2 = *p1;
    // Decrement the number of cells only after checking for Null Pointers
    m->num_elem --;
    // If no more Cells remain in the last Chunk deallocate it.
    #if DEALLOCATE_UNUSED_CHUNKS == TRUE
        if ((m->num_elem % CHUNK_POINTER_IDX) == (CHUNK_POINTER_IDX - 1)) {
            deallocate_last_chunk(m);
        }
    #endif
    return 0;
}

// -------------------------------------------------------------------------- //

// Returns Cell at the given Index, resolving the Linked List.
// Returns Null-Pointer if Index is outside of the allocated Memory Space.
Inner * get_elem (struct MemoryManager m, long idx) {
    // Ensure that the Index can actually point to an initialized Cell.
    // If this check is omitted get_chunk_pointer_inner can return a
    // valid pointer to a Cell which was not yet initialised.
    if ((idx < 0) || (idx >= m.num_elem)) return NULL;
    // Recursively resolve Chunk Pointers until the Cell is found.
    return get_elem_inner(m.chunks, idx);
}

// Private Helper Function for get_elem
static Inner * get_elem_inner (Inner * c, long idx) {
    // Check if the Cell is in the current Chunk
    if (idx < (CHUNK_POINTER_IDX))
        return &(c[idx]);
    // If not check if there is another allocated Chunk
    if (c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD != 0) {
        #if OUTPUT_REDIRECTION == TRUE
            printf("Chunk Redirection to: %p\n", (Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD);
        #endif
        // Recursively resolve Chunk Pointers
        return get_elem_inner((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD, idx-CHUNK_SIZE+1);
    }
    return NULL;
}

// -------------------------------------------------------------------------- //

// Returns the Chunk Pointer with the specified Index
// or the last Chunk Pointer if Index = -1 or bigger
// than the number of allocated Chunks.
Inner * get_chunk_pointer (struct MemoryManager m, long idx) {
    if (idx == 0)
        return m.chunks;
    return get_chunk_pointer_inner(m.chunks, idx - 1);
}

// Private Helper Function for get_chunk_pointer
static Inner * get_chunk_pointer_inner (Inner * c, long idx) {
    if ((idx == 0) || ((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD == NULL)) {
        #if OUTPUT_REDIRECTION == TRUE
            printf("Chunk Redirection to: %p\n", (Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD);
        #endif
        return &c[CHUNK_POINTER_IDX];
    }
    return get_chunk_pointer_inner((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD, idx-1);
}

// -------------------------------------------------------------------------- //

void print_chunks(struct MemoryManager m, char * (*display)(Inner)){

    Inner * chunks = m.chunks;
    long idx;
    long chunk_idx = 0;

    if (m.num_elem <= 0) {
        printf("[]\n");
        return;
    }

    printf("Mem: %p -> [\n", chunks);

    for (long iLauf = 0; iLauf < m.num_elem + chunk_idx; iLauf++) {
        // Get the Index of the current Cell in a Chunk
        idx = iLauf % CHUNK_SIZE;
        // Check if the current Cell is the Chunk-Pointer.
        if (idx == CHUNK_POINTER_IDX) {
            // Break if no more Chunks remain to be printed.
            if ((Inner *) chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD == NULL) break;
            // If more Chunks remain, swap the Chunk Pointer
            chunks = (Inner *) chunks[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD;
            // Increase the Chunk-Index
            chunk_idx ++;
            printf("] -> %p -> [\n", chunks);
            continue;
        }
        printf("\t%-3ld: %p -> %s, \n",
            iLauf - chunk_idx + 1, chunks + idx, display(chunks[idx])
        );
    }

    // The number of Chunks printed to the Console is not always equal
    // to the number of Chunk-Pointers that actually hold the
    // allocated Chunks because the remove_elem-Function does not always
    // automatically deallocate Chunks (too late but never to early).
    printf(
        "] = %ld Elements in %ld Chunks \n",
        m.num_elem, m.allocated_chunks
    );

}

// -------------------------------------------------------------------------- //

void print_chunk_pointers(struct MemoryManager m) {
    Inner * c = m.chunks;

    printf("Chunk Pointers: [");

    while ((Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD != NULL) {
        printf("%p -> ", c);
        c = (Inner *) c[CHUNK_POINTER_IDX].CHUNK_POINTER_FIELD;
    }

    printf("%p]\n", c);
}

// -------------------------------------------------------------------------- //
