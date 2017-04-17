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
#include <string.h>

static void syscall_handler (struct intr_frame *);
static int get_user (const uint8_t *uaddr);
bool verify_user_ptr(void *vaddr, uint8_t argc);

/* System call function prototypes */
void halt(void);
void exit(int status);
pid_t exec(const char*cmd_line);
int wait(pid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

/* Since the return value of the call doesn't necessarily indicate success in executing the system call (i.e. wait), 
   we desynchronize the value stored in the frame pointer's EAX from the success of the call. */
static int exit_wrapper(struct intr_frame *f);
static int exec_wrapper(struct intr_frame *f);
static int wait_wrapper(struct intr_frame *f);
static int create_wrapper(struct intr_frame *f);
static int remove_wrapper(struct intr_frame *f);
static int open_wrapper(struct intr_frame *f);
static int filesize_wrapper(struct intr_frame *f);
static int read_wrapper(struct intr_frame *f);
static int write_wrapper(struct intr_frame *f);
static int seek_wrapper(struct intr_frame *f);
static int tell_wrapper(struct intr_frame *f);
static int close_wrapper(struct intr_frame *f);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Reads a byte at user virtual address uaddr
   uaddr must be below PHYS_BASE.
   Returns the byte value if successful, 
   -1 if the virtual address is >= PHYS_BASE or if segfault occured */

static int
get_user (const uint8_t *uaddr)
{
  if (!is_user_vaddr(uaddr))
    return -1;

  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
        : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
  Returns true if successful, false if the user address is >= PHYS_BASE or if segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if (!is_user_vaddr(udst))
    return false;

  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
        : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

static bool 
is_valid_string (void * str)
{
  int ch = -1;
  while((ch=get_user((uint8_t*)str++))!='\0' && ch!=-1);
    if(ch == '\0')
      return true;
    else
      return false;
}

static void
syscall_handler (struct intr_frame *f) 
{
  int syscall_return_value = -1;

  // Verify that the user provided virtual address is valid
  if (verify_user_ptr (f->esp, 4)) 
  {
    // Retrieve and handle the System call NUMBER on top of the User Stack
    int callNum = *((int*)f->esp);

  	switch (callNum) 
    {
  		case SYS_HALT:
  			halt();
        syscall_return_value = 0;
  			break;
  		case SYS_EXIT:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = exit_wrapper(f); 
        }
  			break;
  		case SYS_EXEC:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = exec_wrapper(f);
        }
  			break;
  		case SYS_WAIT:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = wait_wrapper(f);
        }
  			break;
  		case SYS_CREATE:
        if (verify_user_ptr ((f->esp + 4), 4) && verify_user_ptr ((f->esp + 8), 4))
        {
          syscall_return_value = create_wrapper(f);
        }
  			break;
  		case SYS_REMOVE:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = remove_wrapper(f);
        }
  			break;
  		case SYS_OPEN:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = open_wrapper(f);
        }
        break;
  		case SYS_FILESIZE:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = filesize_wrapper(f);
        }
        break;
      case SYS_READ:
        if (verify_user_ptr ((f->esp + 4), 4) && verify_user_ptr ((f->esp + 8), 4) && verify_user_ptr ((f->esp + 12), 4))
        {
          syscall_return_value = read_wrapper(f);
        }
  			break;
  		case SYS_WRITE:
        if (verify_user_ptr ((f->esp + 4), 4) && verify_user_ptr ((f->esp + 8), 4) && verify_user_ptr ((f->esp + 12), 4))
        {
          syscall_return_value = write_wrapper(f);
        }
  			break;
  		case SYS_SEEK:
        if (verify_user_ptr ((f->esp + 4), 4) && verify_user_ptr ((f->esp + 8), 4))
        {
          syscall_return_value = seek_wrapper(f);
        }
  			break;
  		case SYS_TELL:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = tell_wrapper(f);
        }
  			break;
  		case SYS_CLOSE:
        if (verify_user_ptr ((f->esp + 4), 4))
        {
          syscall_return_value = close_wrapper(f);
        }
  			break;
		  default:
			 break;
    }
  }

  if (syscall_return_value == -1)
  {
    thread_exit (-1);
    return;
  }
}

void 
halt (void) {
  /* Terminates pintos */
	shutdown_power_off();
}

static int
exit_wrapper(struct intr_frame *f)
{
  exit (*((int*)f->esp+1));
  return 0;
}

void 
exit (int status) 
{
  // The actual exit routine will be handled through process_exit (status) called by thread_exit (status).
  thread_exit (status);
}

static int
exec_wrapper(struct intr_frame *f)
{
  char *command_string = *(char **)(f->esp + 4);
  if (!is_valid_string(command_string))
    return -1;

  int length_of_command_string = strlen(command_string);
  if (length_of_command_string >= PGSIZE || length_of_command_string == 0 || command_string[0] == ' ')
    return -1;

  f->eax = exec (command_string);
  return 0;
}

pid_t 
exec (const char* cmd_line)
{
  return process_execute(cmd_line);
}

static int
wait_wrapper(struct intr_frame *f)
{
  f->eax = wait (*((int*)f->esp+1));
  return 0;
}

int 
wait (pid_t pid) 
{
  int status = process_wait(pid); 
  return status;
}

static int
create_wrapper(struct intr_frame *f)
{
  char *file_name_from_frame = (*(char **)(f->esp + 4));
  unsigned initial_size_from_frame = (*(int *)(f->esp + 8));

  if (!is_valid_string(file_name_from_frame))
    return -1;

  f->eax = create(file_name_from_frame, initial_size_from_frame);
  return 0;
}

bool 
create (const char *file, unsigned initial_size) 
{
  bool is_file_created = filesys_create (file, initial_size);
  return is_file_created;
}

static int
remove_wrapper(struct intr_frame *f)
{
  char *file_name_from_frame = *(char **)(f->esp + 4);
  if (!is_valid_string(file_name_from_frame))
    return -1;

  f->eax = remove (file_name_from_frame);
  return 0;
}

bool 
remove (const char *file) 
{
  bool is_file_removed = filesys_remove (file);
  return is_file_removed;
}

static int
open_wrapper(struct intr_frame *f)
{
  char *file_name_from_frame = *(char **)(f->esp + 4);
  if (!is_valid_string(file_name_from_frame))
    return -1;

  f->eax = open(file_name_from_frame);
  return 0;
}

int 
open (const char *file)
{
  struct file *f = filesys_open(file);
  if (f == NULL)
  {
    return -1;
  }

  // Create new file descriptor
  struct fd_elem *fde = malloc (sizeof(struct fd_elem));
  if (fde == NULL)
  {
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

static int
filesize_wrapper(struct intr_frame *f)
{
  int fd = *(int *)(f->esp + 4);
  f->eax = filesize(fd);
  return 0;
}

int 
filesize (int fd) 
{
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fde = find_fd(fd);
    return file_length(fde->file);
  }
  return -1;
}

static int
read_wrapper(struct intr_frame *f)
{
  int fd_from_frame = *(int *)(f->esp + 4);
  void *buffer_from_frame = *(char**)(f->esp + 8);
  unsigned size_from_frame = *(unsigned *)(f->esp + 12);
  if (!verify_user_ptr(buffer_from_frame, 1) || !verify_user_ptr(buffer_from_frame + size_from_frame, 1)) 
  {
    return -1;
  }

  f->eax = read(fd_from_frame, buffer_from_frame, size_from_frame);
  return 0;
}

int 
read (int fd, void *buffer, unsigned size) 
{
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fd_elem = find_fd(fd);
    return file_read(fd_elem->file, buffer, size);
  }
  return -1;
}

static int
write_wrapper(struct intr_frame *f)
{
  int fd_from_frame = *(int *)(f->esp + 4);
  void *buffer_from_frame = *(char**)(f->esp + 8);
  unsigned size_from_frame = *(unsigned *)(f->esp + 12);
  if (!verify_user_ptr(buffer_from_frame, 1) || !verify_user_ptr(buffer_from_frame + size_from_frame, 1)){
    return -1;
  }

  f->eax = write(fd_from_frame, buffer_from_frame, size_from_frame);
  return 0;
}

int 
write (int fd, const void *buffer, unsigned size)
{
  // Write to console
  if (fd == STDOUT_FILENO)
  {
    putbuf((char *)buffer, (size_t)size);
    return (int)size;
  }
  else if (find_fd(fd) != NULL)
  {
    return (int)file_write(find_fd(fd)->file, buffer, size);
  }

  return -1;
}

static int
seek_wrapper(struct intr_frame *f)
{
  int fd_from_frame = *(int *)(f->esp + 4);
  unsigned position_from_frame = *(unsigned *)(f->esp + 8);
  seek(fd_from_frame, position_from_frame);
  return 0;
}

void 
seek (int fd, unsigned position) 
{
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fd_elem = find_fd(fd);
    file_seek(fd_elem->file, position);
  }
}

static int
tell_wrapper(struct intr_frame *f)
{
  int fd = *(int *)(f->esp + 4);
  f->eax = tell (fd);
  return 0;
}

unsigned 
tell (int fd) 
{
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fd_elem = find_fd(fd);
    return file_tell(fd_elem->file);
  }
  return -1;
}

static int
close_wrapper(struct intr_frame *f)
{
  int fd = *(int *)(f->esp + 4);
  close (fd);
  return 0;
}

void 
close (int fd){
  if (find_fd(fd) != NULL)
  {
    struct fd_elem *fd_elem = find_fd(fd);
    file_close(fd_elem->file);
    list_remove(&fd_elem->elem);
    free(fd_elem);
  }
}

bool 
verify_user_ptr (void *vaddr, uint8_t argc) 
{
	bool is_valid = true;

  /* Increment byte-by-byte through the address to ensure validity. */
	for (uint8_t i = 0; i < argc; ++i)
  {
    if (get_user (((uint8_t *)vaddr)+i) == -1)
    {
      is_valid = false;
      break;
    }
  }

	return is_valid;
}