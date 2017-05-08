#include "vm/frame.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"

#define MAX_FRAMES 10

struct frame frames[MAX_FRAMES];

static struct lock scan_lock; /* lock to protect frames traversal */

unsigned frame_ct; /* number of physical frames represented */

unsigned hand; /* clock hand for page eviction algorithm */


/* Frame management function definitions */

void frame_init (void)
{
	lock_init (&scan_lock);
}

void* 
frame_alloc (enum palloc_flags flags, struct page *passed_page)
{
	/* Following suit of palloc_get_page, we use a bitwise AND to determine if PAL_USER is set in flags. */
	if ((flags & PAL_USER) == 0)
		return NULL;

	/* Get a single free page, if one is available, to find a kernel virtual base address for our newly allocated frame. */
	void *free_page = palloc_get_page (flags);

	if (free_page != NULL)
	{
		struct frame *new_frame = malloc(sizeof(struct frame));
		new_frame->base = free_page;
		new_frame->page = passed_page;
		lock_acquire (&scan_lock);
		lock_release (&scan_lock);
	}

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

void frame_free(struct frame *frame)
{
	frame->page = NULL;
}