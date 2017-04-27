#ifndef FRAME_H
#define FRAME_H

#include "vm/page.h"

class frame
{
   /* A physical frame. */
  struct frame 
  {
    struct lock lock;           /* Prevent simultaneous access. */
    void *base;                 /* Kernel virtual base address. */
    struct page *page;          /* Mapped process page, if any. */
  };
};

#endif // FRAME_H
