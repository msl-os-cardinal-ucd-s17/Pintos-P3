#include "page.h"

static bool page_hash_less_function(const struct hash_elem*first, const struct hash_elem*second, void *aux UNUSED) {
    struct page *first_ptr = hash_entry(first, struct page, elem);
    struct page *second_ptr = hash_entry(second, struct page, elem);
    if(first_ptr->addr < second_ptr->addr) {
        return (true);
    } else {
        return (false);
    }

}
static unsigned page_hash_hash_function(const struct hash_elem*el, void*aux UNUSED) {
    struct page* ptr = hash_entry(el, struct page, elem);
    return(hash_int((int) ptr->addr));
}

page_table_intialization(struct hash*ptr) {
    hash_init(ptr,page_hash_has_funct, page_less_func, NULL);
}
