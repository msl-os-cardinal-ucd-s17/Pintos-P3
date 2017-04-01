#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "lib/user/syscall.h"

#define arg0    ((f->esp)-4)
#define arg1	((f->esp)-8)
#define arg2	((f->esp)-12)

static void syscall_handler (struct intr_frame *);
bool verify_user_ptr(void*vaddr);
void system_halt(void);
void system_exit(int status);
pid_t system_exec(const char*cmd_line);


void
syscall_init (void) 
{
	/*Need to Implement Syncronization*/
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //Verify that the user provided virtual address is valid
  if(verify_user_ptr(f->esp)) {
  	  printf ("system call number: %d\n", *((int*) f->esp));

  	//Retrieve and handle the System call NUMBER fromt the User Stack
  	switch(*((int*) f->esp)) {
  		case SYS_HALT:
  			system_halt();
  			break;
  		case SYS_EXIT:
  			system_exit(*((int*)arg0));
  			break;
  		case SYS_EXEC:
  			break;
  		case SYS_WAIT:
  			break;
  		case SYS_CREATE:
  			break;
  		case SYS_REMOVE:
  			break;
  		case SYS_OPEN:
  			break;
  		case SYS_FILESIZE:
  			break;
  		case SYS_READ:
  			break;
  		case SYS_WRITE:
  			break;
  		case SYS_SEEK:
  			break;
  		case SYS_TELL:
  			break;
  		case SYS_CLOSE:
  			break;
		default:
			break;
  	}
  }
  thread_exit ();
}

void system_halt(void) {
	shutdown_power_off();
}

void system_exit(int status) {
	thread_exit();
}

pid_t system_exec(const char*cmd_line){
	pid_t id;
	return(id);
}

bool verify_user_ptr(void *vaddr) {
	bool isValid = 1;
	if(is_user_vaddr(vaddr) && (vaddr < ((void*)LOWEST_USER_VADDR))){
		isValid = 0;	
	}

	return (isValid);
}