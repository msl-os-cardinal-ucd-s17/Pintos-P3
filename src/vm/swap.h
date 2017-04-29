#ifndef SWAP_H
#define SWAP_H

#include <bitmap.h>
#include "devices/block.h"

struct page;


void init_swap(void);
void swap_out(void *frame);
void swap_in(void *frame);

#endif // SWAP_H
