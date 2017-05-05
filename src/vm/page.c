#include "page.h"

bool page_hash_less_function(const struct hash_elem*first, const struct hash_elem*second, void *aux UNUSED) {
    struct page *first_ptr = hash_entry(first, struct page, elem);
    struct page *second_ptr = hash_entry(second, struct page, elem);
    if(first_ptr->addr < second_ptr->addr) {
        return (true);
    } else {
        return (false);
    }

}

void page_hash_action_function(struct hash_elem*el, void *aux UNUSED) {
    struct page*ptr = hash_entry(el, struct page, elem);

    if(ptr->frame != NULL) {
        struct frame *ptr = ptr->frame;
        ptr->frame = NULL;
        //Here we need to free the frame
        //void pseudo_fee_Frame()
    }

    if(ptr->sector != -1) {
        //Function to swap page free
        //void pseudo_swap_free
    }

    //Reclaim memory
    free(ptr);

}
unsigned page_hash_hash_function(const struct hash_elem*el, void*aux UNUSED) {
    struct page* ptr = hash_entry(el, struct page, elem);
    return(hash_int((int) ptr->addr));
}

page_table_intialization(struct hash*ptr) {
    hash_init(ptr,page_hash_has_funct, page_less_func, NULL);
}

struct page* find_page(void*addr) {
    struct thread*curr = thread_current();
    struct page page;
    struct page *found_page = NULL;
    struct hash_elem*elem;

    e = has_find(&curr->pages, &p.hash_elem);

    //Retrieve the entry
    if(e != NULL) {
        found_page = hash_entry(elm, struct page, hash_elem);
    }

    return (found_page);

};
