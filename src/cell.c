
// -------------------------------------------------------------------------- //

#if VARIANT == EASY
    struct Cell {
        bool alive;
        // The Neighbour Count can only be a positive Integer from 0 to 8
        // meaning u8 is sufficiently big.
        u8 neighbours;
    };
#else
    struct Cell {
        // The Neighbour Count can only be a positive Integer from 0 to 8
        // meaning u8 is sufficiently big.
        u8 neighbours;
        long x;
        long y;
    };
#endif

// -------------------------------------------------------------------------- //

#if VARIANT == EASY
    struct Cell alive () {
        struct Cell cell;
        cell.alive = true;
        cell.neighbours = 0;
        return cell;
    }
    struct Cell dead () {
        struct Cell cell;
        cell.alive = false;
        cell.neighbours = 0;
        return cell;
    }
#else
    struct Cell alive (long y, long x) {
        struct Cell cell;
        cell.neighbours = 0;
        cell.x = x;
        cell.y = y;
        return cell;
    }
    // Create a more fitting Alias function
    struct Cell (*new_cell) (long x, long y) = &alive;
#endif

// -------------------------------------------------------------------------- //
