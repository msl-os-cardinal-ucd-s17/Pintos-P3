#include "swap.h"

/* Global swap device */
struct block *swap_block;

/* Global bitmap to keep track of open slots */
struct bitmap *swap_bitmap;

/* Global lock to protect the swap bitmap */
struct lock swap_lock;

/* Number of sectors per page */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

/* Bit for each slot */
#define SWAP_EMPTY 0
#define SWAP_FULL 1
