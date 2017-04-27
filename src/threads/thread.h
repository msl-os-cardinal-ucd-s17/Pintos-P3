#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/synch.h"
#include "vm/page.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. Can only be modified by thread_set_priority. */
    int effective_priority;             /* Priority based on donors; can be safely referenced even when ignoring donation. */

    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c */
    struct list_elem elem;              /* List element. References an element in the ready_list OR the semaphore waiting list, never both. */
    struct list_elem sleep_elem;
    struct semaphore sleep_sema;
    struct lock *blocking_lock;         /* Points to the lock acquired within synch.c */

    struct list donor_list;             /* List of donor threads; each list_elem is a donor_elem of another thread. */
    struct list_elem donor_elem;        /* List element that allows this thread to be referenced in another thread's donor_list. */

    /* MLFQS data members */
    int nice; 				                  /* Nice value */
    int recent_cpu;			                /* Recent CPU */

    // Value of OS ticks when the thread should wake up
    int64_t wake_up_time;

    // ****************************************************************

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
    struct list fd_list;                /* List of open files. */
    int fd_count;                       /* Open file counter. */

    char *program_name;         /* Name of program intended to be run as a process */
    struct file *executable;    /* File structure for executable user program */

    struct list child_list;   /* List of children - use for syscall synchronization */
#endif

    /*Pages owned by the thread*/
    struct hash *pages;                 /* Page table. */

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (int) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);
void thread_donate_priority (void);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);
void calc_load_avg (void);
void calc_recent_cpu (struct thread *t);
void increment_recent_cpu (void);

void m_priority (struct thread *);
void test_sleeping_thread (int64_t);
void add_sleeping_thread (struct thread *);

void add_thread_ready_priority_list (struct thread*);
void add_thread_sema_priority_list (struct thread*, struct semaphore*);
void sort_thread_sema_priority_list (struct semaphore*);
void verify_current_thread_highest (void);
void recalc_mlfqs (void);
bool thread_priority_less (const struct list_elem *, const struct list_elem *, void *);
void thread_priority_synchronize (void);

struct thread *get_thread (tid_t tid); /* Get thread by tid from all_list */
struct thread *get_child (tid_t tid); /* Get thread by tid from thread's child_list */

int returnLoadAverage(void);

#endif /* threads/thread.h */
