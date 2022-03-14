
// -------------------------------------------------------------------------- //

#if VARIANT == 0
    struct Cell {
        bool alive;
        // The Neighbour Count can only be a positive Integer from 0 to 8
        // meaning u8 is sufficiently big.
        u8 neighbours;
    };
#else
    struct Cell {
        bool alive;
        // The Neighbour Count can only be a positive Integer from 0 to 8
        // meaning u8 is sufficiently big.
        u8 neighbours;
        long x;
        long y;
    };
#endif

// -------------------------------------------------------------------------- //

#if VARIANT == 0
    struct Cell alive () {
        struct Cell cell;
        cell.alive = true;
        cell.neighbours = 0;
        return cell;
    }
#else
    struct Cell alive (long x, long y) {
        struct Cell cell;
        cell.alive = true;
        cell.neighbours = 0;
        cell.x = x;
        cell.y = y;
        return cell;
    }
#endif

// -------------------------------------------------------------------------- //

#if VARIANT == 0
    struct Cell dead () {
        struct Cell cell;
        cell.alive = false;
        cell.neighbours = 0;
        return cell;
    }
#else
    struct Cell dead (long x, long y) {
        struct Cell cell;
        cell.alive = false;
        cell.neighbours = 0;
        cell.x = x;
        cell.y = y;
        return cell;
    }
#endif

// -------------------------------------------------------------------------- //
