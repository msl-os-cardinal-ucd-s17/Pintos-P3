#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <hash.h>
#include "threads/palloc.h"
#include "filesys/file.h"
#include "vm/frame.h"
#include "threads/synch.h"
#include "devices/block.h"


/* Virtual page. */
struct page
  {
    /* Immutable members. */
    void *addr;                 /* User virtual address. */
    bool read_only;             /* Read-only page? */
    struct thread *thread;      /* Owning thread. */

    /* Accessed only in owning process context. */
    struct hash_elem hash_elem; /* struct thread `pages' hash element. */

    /* Set only in owning process context with frame->lock held.
       Cleared only with scan_lock and frame->lock held. */
    struct frame *frame;        /* Page frame. */

    /* Swap information, protected by frame->lock. */
    block_sector_t sector;       /* Starting sector of swap area, or -1. */

    /* Memory-mapped file information, protected by frame->lock. */
    bool private;               /* False to write back to file,
	                           true to write back to swap. */
    struct file *file;          /* File. */
    off_t file_offset;          /* Offset in file. */
    off_t file_bytes;           /* Bytes to read/write, 1...PGSIZE. */
  };

void page_table_intialization(struct hash*ptr);
bool page_hash_less_function(const struct hash_elem*first, const struct hash_elem*second, void *aux UNUSED);
void page_hash_action_function(struct hash_elem*el, void *aux UNUSED);
unsigned page_hash_hash_function(const struct hash_elem*el, void*aux UNUSED);
struct page*find_page(void*addr);



#endif // PAGE_H
