#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "lib/user/syscall.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

#define arg0  ((f->esp)+4)
#define arg1	((f->esp)+8)
#define arg2	((f->esp)+12)

static void syscall_handler (struct intr_frame *);
static int get_user (const uint8_t *uaddr);
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

#define ARG_MAX 3

int syscall_args[ARG_MAX]; // three int array, for max number of arguments in syscall

bool get_args(struct intr_frame *f, int *args, int argc);

typedef struct fd_elem fd_entry;

struct fd_elem
{
   int fd;
   struct file* file;
   struct list_elem elem;
};

struct lock file_lock; // lock for synchronizing access to filesys functions 

// Check current thread's list of open files for fd
static struct fd_elem* find_fd(int fd)
{
   struct list_elem *e;
   struct fd_elem *fde = NULL;
   struct list *fd_elems = &thread_current()->fd_list;

   for (e = list_begin(fd_elems); e != list_end(fd_elems); e = list_next(e))
   {
      struct fd_elem *t = list_entry (e, struct fd_elem, elem);
      if (t->fd == fd)
      {
         fde = t;
         break;
      }
   }

   return fde;
}

static int 
next_fd (void)
{
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
   occured */

static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
        : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
UDST must be below PHYS_BASE.
Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
        : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

static void
syscall_handler (struct intr_frame *f) 
{
  int syscall_return_value = -1;
  bool valid_user_args = false;
  // Verify that the user provided virtual address is valid
  if (verify_user_ptr (f->esp)) 
  {
    // Retrieve and handle the System call NUMBER fromt the User Stack
    int callNum = *((int*)f->esp);

  	switch (callNum) 
    {
  		case SYS_HALT:
  			system_halt();
        syscall_return_value = 0;
  			break;
  		case SYS_EXIT:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          system_exit(syscall_args[0]);
          syscall_return_value = 0; 
        }
  			break;
  		case SYS_EXEC:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = (int) system_exec((char *)syscall_args[0]);
        }
  			break;
  		case SYS_WAIT:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = system_wait(*(pid_t*)syscall_args[0]);
        }
  			break;
  		case SYS_CREATE:
        valid_user_args = get_args(f, syscall_args, 2);
        if (valid_user_args)
        {
          syscall_return_value = (int) system_create((char*)syscall_args[0], (unsigned) syscall_args[1]);
        }
  			break;
  		case SYS_REMOVE:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = (int) system_remove((char*) syscall_args[0]);
        }
  			break;
  		case SYS_OPEN:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = system_open((char*) syscall_args[0]);
        }
        break;
  		case SYS_FILESIZE:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = system_filesize(syscall_args[0]);
        }
        break;
      case SYS_READ:
        valid_user_args = get_args(f, syscall_args, 3);
        if (valid_user_args)
        {
          syscall_return_value = system_read(syscall_args[0], (void *)syscall_args[1], (unsigned)syscall_args[2]);
        }
  			break;
  		case SYS_WRITE:
        valid_user_args = get_args(f, syscall_args, 3);
        if (valid_user_args)
        {
          syscall_return_value = system_write(syscall_args[0], (void*)syscall_args[1], (unsigned)syscall_args[2]);
        }
  			break;
  		case SYS_SEEK:
        valid_user_args = get_args(f, syscall_args, 2);
        if (valid_user_args)
        {
          system_seek(syscall_args[0], (unsigned)syscall_args[1]); 
          syscall_return_value = 0;
        }
  			break;
  		case SYS_TELL:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          syscall_return_value = (int) system_tell(syscall_args[0]);
        }
  			break;
  		case SYS_CLOSE:
        valid_user_args = get_args(f, syscall_args, 1);
        if (valid_user_args)
        {
          system_close(syscall_args[0]);
          syscall_return_value = 0;
        }
  			break;
		  default:
			 break;
    }
  }

  if (syscall_return_value == -1)
  {
    thread_exit();
  }

  f->eax = syscall_return_value;
}

void system_halt(void) {
  /* Terminates pintos */
	shutdown_power_off();
}

void system_exit(int status) {
  struct thread *t = thread_current();
  printf("%s: exit(%d)\n", t->name, status);

  struct list_elem *e;

  /* Check for dead children*/
  for (e = list_begin(&t->child_list); e != list_end(&t->child_list);)
  {
    struct thread *child = list_entry(e, struct thread, child_elem);
    child->parent_alive = false;
    /* If child isn't alive, free memory */
    if (!child->alive){
      e = list_remove(&child->child_elem);
      free(child);
    }
    else
    {
      e = list_next(e);
    }
  }
  
  /* If parent is alive, update thread's status and signal parent with sema_up */
  if (t->parent_alive)
  {
    t->alive = false;
    t->exit_status = status;
    sema_up(&t->wait_sema);
  }
  else 
  {
    /* If parent is dead, free memory */
    list_remove(&t->child_elem);
    free(t);
  }

  /* Free files */
  for (e = list_begin(&t->fd_list); e != list_end(&t->fd_list);)
  {
    struct fd_elem *fde = list_entry(e, struct fd_elem, elem);
    file_close(fde->file);
    e = list_remove(&fde->elem);
    free(fde);
  }

  /* Lastly, call thread exit */
  thread_exit();
}

pid_t system_exec(const char* cmd_line){
  /* runs executable given by cmd_line */
  pid_t process_thread_id = process_execute(cmd_line);
  return process_thread_id;
}

int system_wait(pid_t pid) {
  int status = process_wait(pid); 
  return status;
}

bool system_create(const char *file, unsigned initial_size) {
  // need to check that file is a valid user address
  
  lock_acquire(&file_lock);
  bool fileCreate = filesys_create (file, (off_t)initial_size);
  lock_release(&file_lock);
  return fileCreate;
}

bool system_remove(const char *file) {
  // need to check file is valid user address
  lock_acquire(&file_lock);
  bool fileRemove = filesys_remove(file);
  lock_release(&file_lock);
  return fileRemove;
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
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fde = find_fd(fd);
    return file_length(fde->file);
  }
  return -1;
}

int system_read(int fd, void *buffer, unsigned size) {
  // If fd == 0, read from keyboard
  lock_acquire(&file_lock);
  if (fd == STDIN_FILENO) {
    uint8_t *tmp_buffer = (uint8_t *) buffer;
    for(unsigned i = 0; i < size; i++) {
      tmp_buffer[i] = input_getc();
      // (uint8_t *) buffer[i] = input_getc();
    }
    return size;
  }
  
  // Otherwise read from a file
  else 
  {
    struct file* readFile = find_fd(fd)->file;
    
    if (readFile != NULL) {
      int bytesRead = file_read(readFile, buffer, size);
      lock_release(&file_lock);
      return bytesRead;
    }
    else {
      return -1; // file not found
    }
  }
}

int system_write(int fd, const void *buffer, unsigned size){
   // Write to console
   if (fd == STDOUT_FILENO)
   {
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
   if (find_fd(fd) != NULL)
   {
      int bytes_written = file_write(find_fd(fd)->file, buffer, size);
      return bytes_written;
   }
   else 
   {
      // File not found
      return -1;
   }
}

void system_seek(int fd, unsigned position) {
  struct file* file = find_fd(fd)->file;
  
  lock_acquire(&file_lock);
  file_seek (file, (off_t)position);
  lock_release(&file_lock);  
}

unsigned system_tell(int fd) {
  struct file* file = find_fd(fd)->file;
  
  lock_acquire(&file_lock);
  unsigned position = (unsigned)file_tell (file);
  lock_release(&file_lock);
  
  return position;
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

bool 
verify_user_ptr (void *vaddr) 
{
	bool isValid = 1;
	if(is_user_vaddr(vaddr) && (vaddr < ((void*)LOWEST_USER_VADDR))){
		isValid = 0;	
	}

	return (isValid);
}

/* Retrieve arguments from stack */
bool
get_args (struct intr_frame *f, int *args, int argc) 
{
  int *argptr; // pointer to argument
  bool valid_args = true;
  for (int i = 0; i < argc; ++i) 
  {
    argptr = (int*)f->esp + (4 * i);
    if (verify_user_ptr((void *)argptr)) 
    {
      args[i] = *argptr; // put the contents of argument pointer into array or args
    }
    else 
    {
      valid_args = false;
      break;
    }
  }

  return valid_args;
}
