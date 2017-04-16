#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

void process_initialize_lists (void);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (int);
void process_activate (void);

struct fd_elem
{
   int fd;
   struct file* file;
   struct list_elem elem;
};

struct process_id 
{
	int pid;
	struct list_elem elem;
};

struct fd_elem* find_fd (int fd);
int next_fd (void);

#endif /* userprog/process.h */
