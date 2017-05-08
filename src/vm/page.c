#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "threads/interrupt.h"

bool 
page_hash_less_function (const struct hash_elem*first, const struct hash_elem*second, void *aux UNUSED) 
{
    struct page *first_ptr = hash_entry(first, struct page, elem);
    struct page *second_ptr = hash_entry(second, struct page, elem);
    if (first_ptr->addr < second_ptr->addr)
    {
        return true;
    }

    return false;
}

void 
page_hash_action_function(struct hash_elem*el, void *aux UNUSED) {
    struct page *page_ptr = hash_entry (el, struct page, elem);

    if (page_ptr->frame != NULL) 
    {
        frame_free (pagedir_get_page (thread_current()->pagedir, page->addr));
        pagedir_clean_page (thread_current()->pagedir, page->addr);
    }

    free (page_ptr);

}

unsigned 
page_hash_hash_function (const struct hash_elem*el, void*aux UNUSED) 
{
    struct page* ptr = hash_entry(el, struct page, elem);
    return(hash_int((int) ptr->addr));
}

void
page_table_intialization (struct hash *ptr) 
{
    hash_init (ptr, page_hash_hash_function, page_hash_less_function, NULL);
}

void
page_table_destroy (struct hash *ptr) 
{
    hash_destroy (ptr, page_hash_action_function);
}


struct page * 
find_page (void *addr) 
{
    struct thread *curr = thread_current();
    struct page page;
    struct page *found_page = NULL;
    page.addr = pg_round_down (addr);

    struct hash_elem *elem;

    e = hash_find(&curr->pages, &p.hash_elem);

    // Retrieve the entry
    if (!e) 
    {
        found_page = hash_entry(e, struct page, hash_elem);
    }

    return found_page;
};

bool
stack_grow (void *user_addr)
{
    if ((size_t) (PHYS_BASE - pg_round_down(user_addr)) > STACK_LIMIT)
        return false;

    struct page *new_page = malloc(sizeof(struct page));
    if (!new_page)
        return false;

    new_page->addr = pg_round_down(user_addr);
    new_page->thread = thread_current ();
    new_page->read_only = false;

    /* TODO: Install page to frame */
    void *frame_for_new_page = NULL; // Replace with allocated frame
    if (frame_for_new_page != NULL)
    {
        free(new_page);
        return false;
    }

    if (!install_page(new_page->addr, frame_for_new_page, new_page->read_only))
    {
        free(new_page);
        // TODO: Free frame_for_new_page
        return false;
    }

    return (hash_insert(&thread_current()->pages, &new_page->hash_elem) == NULL);
}