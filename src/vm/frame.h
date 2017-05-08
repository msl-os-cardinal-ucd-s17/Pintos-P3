#ifndef FRAME_H
#define FRAME_H

#include "vm/page.h"
#include "threads/palloc.h"

/* A physical frame. */
struct frame 
{
	struct lock lock;           /* Prevent simultaneous access. */
	void *base;                 /* Kernel virtual base address. */
	struct page *page;          /* Mapped process page, if any. */
};

#endif // FRAME_H

/* Frame management function declarations */
void frame_init (void);
void* frame_alloc (enum palloc_flags, struct page *);
void frame_lock (struct page *page);
void frame_unlock (struct page *page);
void frame_free (struct frame *frame);
