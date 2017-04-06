#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "lib/user/syscall.h"

#define arg0  ((f->esp)+4)
#define arg1	((f->esp)+8)
#define arg2	((f->esp)+12)
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "filesys/filesys.h"
static void syscall_handler (struct intr_frame *);
// static int get_user (const uint8_t *uaddr);
bool verify_user_ptr(void*vaddr);

/* System call function prototypes */
void system_halt(void);
void system_exit(int status);
pid_t system_exec(const char*cmd_line);
int system_wait(pid_t pid);
bool system_create(const char *file, unsigned initial_size);
bool system_remove(const char *file);
int system_open(const char *file);
int system_filesize(int fd);
int system_read(int fd, void *buffer, unsigned size);
int system_write(int fd, const void *buffer, unsigned size);
void system_seek(int fd, unsigned position);
unsigned system_tell(int fd);
void system_close(int fd);

static struct fd_elem* find_fd(int fd);
static int next_fd(void);

typedef struct fd_elem fd_entry;

struct fd_elem{
   int fd;
   struct file* file;
   struct list_elem elem;
};


// Check current thread's list of open files for fd
static struct fd_elem* find_fd(int fd){
   struct list_elem *e;
   struct fd_elem *fde = NULL;
   struct list *fd_elems = &thread_current()->fd_list;

   for (e = list_begin(fd_elems); e != list_end(fd_elems); e = list_next(e)){
      struct fd_elem *t = list_entry (e, struct fd_elem, elem);
      if (t->fd == fd){
         fde = t;
         break;
      }
   }

   return fde;
}

static int next_fd(void){
   return thread_current()->fd_count++;
}


void
syscall_init (void) 
{
	/*Need to Implement Syncronization*/
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Reads a byte at user virtual address uaddr
   uaddr must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occured
 */

// static int get_user (const uint8_t *uaddr)
// {
//   int result;
//   asm ("movl $1f, %0; movzbl %1, %0; 1:"
//         : "=&a" (result) : "m" (*uaddr));
//   return result;
// }

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //Verify that the user provided virtual address is valid
  if(verify_user_ptr(f->esp)) {
    int callNum; // set up a local variable to hold the call number
    callNum = *((int*)f->esp);

  	printf ("system call number: %d\n", callNum);
  	//Retrieve and handle the System call NUMBER fromt the User Stack
  	switch(callNum) {
  		case SYS_HALT:
  			system_halt();
  			break;
  		case SYS_EXIT:
  			system_exit(*(int*)arg0);
  			break;
  		case SYS_EXEC:
        system_exec((char*)arg0);
  			break;
  		case SYS_WAIT:
        system_wait(*(pid_t*)arg0);
  			break;
  		case SYS_CREATE:
        system_create((char*)arg0, *(unsigned*)arg1);
  			break;
  		case SYS_REMOVE:
        system_remove((char*)arg0);
  			break;
  		case SYS_OPEN:
        system_open((char*)arg0);//(const char *file);
  			break;
  		case SYS_FILESIZE:
        system_filesize(*(int*)arg0);
        break;
      case SYS_READ:
        system_read(*(int*)arg0, arg1, *(unsigned*)arg2);
  			break;
  		case SYS_WRITE:
        system_write(*((int*)arg0), arg1, *((unsigned*)arg2));
  			break;
  		case SYS_SEEK:
        system_seek(*(int*)arg0, *(unsigned*)arg1);    
  			break;
  		case SYS_TELL:
        system_tell(*(int*)arg0);
  			break;
  		case SYS_CLOSE:
        system_close(*(int*)arg0); //(int fd);
  			break;
		default:
			break;
    }
  }
  thread_exit ();
}

void system_halt(void) {
  /* Terminates pintos */
	shutdown_power_off();
}

void system_exit(int status) {
    // printf("Status Number: %d\n", status);
    /* Terminates current user program and returns status to the kernel */
    thread_exit();
}

pid_t system_exec(const char*cmd_line){
  /* runs executable given by cmd_line */
  pid_t id;
  return(id);
}

int system_wait(pid_t pid) {
  printf("sys_wait not implemented");
}

bool system_create(const char *file, unsigned initial_size) {
  printf("sys_create not implemented");

}

bool system_remove(const char *file) {
  printf("sys_remove not implemented");
}

int system_open(const char *file){
   struct file *f = filesys_open(file);
   if (f == NULL){
      return -1;
   }

   // Create new file descriptor
   struct fd_elem *fde = malloc (sizeof(struct fd_elem));
   if (fde == NULL){
      return -1;
   }

   // Increment file descriptor
   fde->fd = next_fd(); 
   // Save file with fd struct
   fde->file = f;
   // Add to threads open file list
   list_push_back(&thread_current()->fd_list, &fde->elem);

   // Return file descriptor
   return fde->fd;

}

int system_filesize(int fd) {
  printf("sys_filesize not implemented");
}

int system_read(int fd, void *buffer, unsigned size) {
  printf("sys_read not implemented");
}

int system_write(int fd, const void *buffer, unsigned size){
   // Write to console
   if (fd == STDOUT_FILENO){
      if (size <= 300){
         putbuf((char *)buffer, (size_t) size);
      }
      else {
         unsigned t_size = size;
         while (t_size > 300){
            putbuf((char *)buffer, (size_t) t_size);
            t_size -= 300;
         }
         putbuf((char *)buffer, (size_t) t_size);
      }
      return (int)size;
   }
   // Write to file
   if (find_fd(fd) != NULL){
      int bytes_written = file_write(find_fd(fd)->file, buffer, size);
      return bytes_written;
   }
   else {
      // File not found
      return -1;
   }
}

void system_seek(int fd, unsigned position) {
  printf("sys_seek not implemented");
}

unsigned system_tell(int fd) {
  printf("sys_tell not implemented");
}

void system_close(int fd){
   if (find_fd(fd) != NULL){
      struct fd_elem *fde = find_fd(fd);
      // Close file
      file_close(fde->file);
      // Remove from open file list
      list_remove(&fde->elem);
      // Free memory
      free(fde);
   }
}

bool verify_user_ptr(void *vaddr) {
	bool isValid = 1;
	if(is_user_vaddr(vaddr) && (vaddr < ((void*)LOWEST_USER_VADDR))){
		isValid = 0;	
	}

	return (isValid);
}

