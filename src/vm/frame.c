#include "frame.h"
#include "threads/synch.h"


#define NUM_FRAMES 10

frame frames[NUM_FRAMES]; /* global frames array */

static struct lock scan_lock; /* lock to protect frames traversal */

unsigned frame_ct; /* number of physical frames represented */

unsigned hand; /* clock hand for page eviction algorithm */


/* Frame management function definitions */
void frame_init(void)
{
	// -- obtain physical frames from user pool using palloc_get_page  (PAL_USER)
	// -- use malloc to allocate memory for each struct frame that comes from the
	// 	  page allocator
	// -- initialize frame lock
}

/* Acquires frame lock for given page */
void frame_lock(struct page *page)
{
	struct frame *page_frame;
	page_frame = page->frame;

	if (page_frame != NULL)
	{
		lock_acquire(page_frame->lock);
	}
}


/* Releases frame_lock of given page */
void frame_unlock(struct page *page)
{
	struct frame *page_frame;
	page_frame = page->frame;

	if (page_frame != NULL)
	{
		lock_release(page_frame->lock);
	}
}

void free_frame(struct frame *frame)
{
	frame->page = NULL;
}