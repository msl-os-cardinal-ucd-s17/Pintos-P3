#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <hash.h>
#include "threads/palloc.h"
#include "filesys/file.h"
#include "vm/frame.h"
#include "threads/synch.h"


class page
{
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

};

#endif // PAGE_H
