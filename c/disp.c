/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <i386.h>
#include <xeroslib.h>
#include <stdarg.h>

static int  kill(pcb *currP, int pid);

static pcb      *head = NULL;
static pcb      *tail = NULL;

void     dispatch( void ) {
/********************************/

    pcb				*p;
    int				r;
    funcptr			fp;
    int				stack;
    va_list			ap;
    char			*str;
    int				len;

	int				pid;
	int				signum;
	pcb*			temp_p;
	int				dvnum;
	int				fd;
	void*			buff;
	int				bufflen;
	unsigned long	command;


    for( p = next(); p; ) {
      //      kprintf("Process %x selected stck %x\n", p, p->esp);

      r = contextswitch( p );
      switch( r ) {
      case( SYS_CREATE ):
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg( ap, int ) );
        stack = va_arg( ap, int );
		p->ret = create( fp, stack );
        break;
      case( SYS_YIELD ):
        ready( p );
        p = next();
        break;
      case( SYS_STOP ):
        p->state = STATE_STOPPED;
        p = next();
        break;
      case (SYS_CPUTIMES):
		ap = (va_list) p->args;
		p->ret = getCPUtimes(p, va_arg(ap, processStatuses *));
		break;
      case( SYS_PUTS ):
		ap = (va_list)p->args;
		str = va_arg( ap, char * );
		kprintf( "%s", str );
		p->ret = 0;
		break;
      case( SYS_GETPID ):
		p->ret = p->pid;
		break;
      case( SYS_SLEEP ):
		ap = (va_list)p->args;
		len = va_arg( ap, int );
		sleep( p, len );
		p = next();
		break;
      case( SYS_TIMER ):
		tick();
		//kprintf("T");
		p->cpuTime++;
		ready( p );
		p = next();
		end_of_intr();
		break;

/************************/
/* 2.3 syscall handling */
/************************/

	  // handler for syssighandler system call
      case ( SYS_SIGHANDLER ):
        ap = (va_list) p->args;
		signum = va_arg( ap, int );
		funcptr  newhandler = (funcptr)( va_arg( ap, int ) );
		funcptr* oldhandler = (funcptr*)( va_arg( ap, int* ) );

		// the code checks if the passed signal is valid OR
		// if the signal that are previously passed is signal 31 (the highest priority possible)
		// if so, the signal is considered invalid and we return -1
		if( signum < 0  ||
			signum > 31 ||
			// assume oldhandler were passed correctly
			// and there exists a previously installed handler for signal 31 and they match
			// this means we are trying to interrupt signal 31 which we do not allow
			(funcptr*)oldhandler == (funcptr*)p->signalTable[31].handler ) {
				kprintf( "invalid signal %d\n", signum );
				p->ret = -1;
				break;
		}

		// (check handler address' validity)
		// (if invalid, return -2)
		// (we are unsure of what to do here...)

		// if the signal is validated and passed, we install the handler
		// then, assume again the oldhandler were passed correctly, we just set
		// our oldhandler pointer points to the passed oldhandler.
		p->signalTable[signum].handler = newhandler;
		p->signalTable[signum].oldhandler = oldhandler;
		kprintf( "new handler installed\n" );

		// handler successfully installed, we return 0
		p->ret = 0;
		break;

	  // handler to handle syssigreturn system call (should only be used by tramp code)
      case ( SYS_SIGRETURN ):
        ap = (va_list) p->args;
		long old_sp = (long) va_arg( ap, void* );

		// simply replace the stack pointer to point to the preset old stack pointer
		p->esp = old_sp;
		break;

	  // handler to handle syskill system call
	  // functionality changed to be passing signals instead of killing a process from a2
      case ( SYS_KILL ):
        ap = (va_list) p->args;
		pid = va_arg( ap, int );
		signum = va_arg( ap, int );

		// checks if the passed signal number is valid
		// if the signal is invalid, return -561
		if( signum < 0 || signum > 31 ) {
			kprintf( "invalid signal %d\n", signum );
			p->ret = -561;
			break;
		}

		temp_p = getProcess( pid );
		
		// if signal is valid but the target process doesn't exists then we return -512
		if( temp_p == NULL ) {
			kprintf( "target process id %d does not exist\n", pid );
			p->ret = -512;
			break;
		}

		// otherwise, the signal marking and delivery takes place
		// and we'll have signal() to set the return code for us
		p->ret = signal( pid, signum );
		kprintf( "signal marked successfully\n" );
		break;

	  // handler for syswait system call
	  // terminates the targeted process
      case ( SYS_WAIT ):
        ap = (va_list) p->args;
		pid = va_arg( ap, int );
		p->ret = kill( p, pid );
		p = next();
		break;

/************************/
/* 2.3 syscall handling */
/************************/

	  // handler for sysopen system call, which relies on di_calls to communicate with devices
      case ( SYS_OPEN ):
		ap = (va_list) p->args;
		dvnum = va_arg( ap, int );

		p->ret = di_open( p, dvnum );
		break;

	  // handler for sysclose system call, which relies on di_calls to communicate with devices
      case ( SYS_CLOSE ):
		ap = (va_list) p->args;
		fd = va_arg( ap, int );

		p->ret = di_close( p, fd );
		break;

	  // handler for syswrite system call, which relies on di_calls to communicate with devices
      case ( SYS_WRITE ):
		ap = (va_list) p->args;
		fd = va_arg( ap, int );
		buff = va_arg( ap, void* );
		bufflen = va_arg( ap, int );

		p->ret = di_write( p, fd, buff, bufflen );
		break;

	  // handler for sysread system call, which relies on di_calls to communicate with devices
      case ( SYS_READ ):
		ap = (va_list) p->args;
		fd = va_arg( ap, int );
		buff = va_arg( ap, void* );
		bufflen = va_arg( ap, int );

		p->ret = di_read( p, fd, buff, bufflen );
		break;

	  // handler for sysioctl system call, which relies on di_calls to communicate with devices
      case ( SYS_IOCTL ):
		ap = (va_list) p->args;
		fd = va_arg( ap, int );
		command = va_arg( ap, unsigned long );

		p->ret = di_ioctl( p, fd, command );
		break;


      default:
        kprintf( "Bad Sys request %d, pid = %d\n", r, p->pid );
      }
    }

    kprintf( "Out of processes: dying\n" );
    
    for( ;; );
}

extern void dispatchinit( void ) {
/********************************/

  //bzero( proctab, sizeof( pcb ) * MAX_PROC );
  memset(proctab, 0, sizeof( pcb ) * MAX_PROC);
}



extern void     ready( pcb *p ) {
/*******************************/

    p->next = NULL;
    p->state = STATE_READY;

    if( tail ) {
        tail->next = p;
    } else {
        head = p;
    }

    tail = p;
}

extern pcb      *next( void ) {
/*****************************/

    pcb *p;

    p = head;

    if( p ) {
        head = p->next;
        if( !head ) {
            tail = NULL;
        }
    } else { // Nothing on the ready Q and there should at least be the idle proc.
        kprintf( "BAD\n" );
        for(;;);
    }

    p->next = NULL;
    p->prev = NULL;
    return( p );
}


// This function takes a pointer to the pcbtab entry of the currently active process. 
// The functions purpose is to remove the process being pointed to from the ready Q
// A similar function exists for the management of the sleep Q. Things should be re-factored to 
// eliminate the duplication of code if possible. There are some challenges to that because
// the sleepQ is a delta list and something more than just removing an element in a list
// is being preformed. 


void removeFromReady(pcb * p) {

  if (!head) {
    kprintf("Ready queue corrupt, empty when it shouldn't be\n");
    return;
  }

  if (head == p) { // At front of list
    // kprintf("Pid %d is at front of list\n", p->pid);
    head = p->next;

    // If the implementation has idle on the ready list this next statement
    // isn't needed. However, it is left just in case someone decides to 
    // change things so that the idle process is kept separate. 

    if (head == NULL) { // If the implementation has idle process  on the 
      tail = head;      // ready list this should never happen
      kprintf("Kernel bug: Where is the idle process\n");
    }
  } else {  // Not at front, find the process.
    pcb * prev = head;
    pcb * curr;
    
    for (curr = head->next; curr!=NULL; curr = curr->next) {
      if (curr == p) { // Found process so remove it
	// kprintf("Found %d in list, removing\n", curr->pid);
	prev->next = p->next;
	if (tail == p) { //last element in list
	    tail = prev;
	    // kprintf("Last element\n");
	}
	p->next = NULL; // just to clean things up
	break;
      }
      prev = curr;
    }
    if (curr == NULL) {
      kprintf("Kernel bug: Ready queue corrupt, process %d claimed on queue and not found\n",
	      p->pid);
      
    }
  }
}

// This function takes 2 paramenters:
//  currP  - a pointer into the pcbtab that identifies the currently running process
//  pid    - the proces ID of the process to be killed.
//
// Note: this function needs to be augmented so that it delivers a kill signal to a 
//       a particular process. The main functionality of the this routine will remain the 
//       same except that when the process is located it needs to be put onto the readyq
//       and a signal needs to be marked for delivery. 
//

static int  kill(pcb *currP, int pid) {
  pcb * targetPCB;
  
  kprintf("Current pid %d Killing %d\n", currP->pid, pid);
  
  if (pid == currP->pid) {   // Trying to kill self
    return -2;
  }

  // Don't let it kill the idle process, which from the user side
  // of things isn't a real process
  // IDLE process had PID 0

  if (pid == 0) {
    return -1;
  }
    
  if (!(targetPCB = getProcess( pid ))) {
    // kprintf("Target pid not found\n");
    return -1;
  }

  if (targetPCB->state == STATE_STOPPED) {
    kprintf("Target pid was stopped\n");
    return  -1;
  }
  
  // PCB has been found,  and the proces is either sleepign or running.
  // based on that information remove the process from 
  // the appropriate queue/list.

  if (targetPCB->state == STATE_SLEEP) {
    // kprintf("Target pid %d sleeping\n", targetPCB->pid);
    removeFromSleep(targetPCB);
  }

  if (targetPCB->state == STATE_READY) {
    // remove from ready queue
    // kprintf("Target pid %d is ready\n", targetPCB->pid);
    removeFromReady(targetPCB);
  }

  // Check other states and do state specific cleanup before stopping
  // the process 
  // In the new version the process will not be marked as stopped but be 
  // put onto the readyq and a signal marked for delivery. 

  targetPCB->state = STATE_STOPPED;
  return 0;
}
  

// This function is the system side of the sysgetcputimes call.
// It places into a the structure being pointed to information about
// each currently active process. 
//  p - a pointer into the pcbtab of the currently active process
//  ps  - a pointer to a processStatuses structure that is 
//        filled with information about all the processes currently in the system
//

extern char * maxaddr;
  
int getCPUtimes(pcb *p, processStatuses *ps) {
  
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) 
    return -1;

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(processStatuses)) > maxaddr)  
    return -2;

  // There are probably other address checks that can be done, but this is OK for now


  for (i=0; i < MAX_PROC; i++) {
    if (proctab[i].state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = proctab[i].pid;
      ps->status[currentSlot] = p->pid == proctab[i].pid ? STATE_RUNNING: proctab[i].state;
      ps->cpuTime[currentSlot] = proctab[i].cpuTime * MILLISECONDS_TICK;
    }
  }

  return currentSlot;
}

// code grabed from our a2 assignment to our own convenience
extern pcb *getProcess( int pid ) {

	pcb *p;

	// loop through our process list, return the process if found
	for( int i = 0; i < MAX_PROC; i++ ) {
		if( proctab[i].pid == pid ) {
			kprintf( "process found\n" );
			p = &proctab[i];
			return p;
		}
	}

	// return NULL if there are no process at all, or if no process with such PID exists
	kprintf( "process with requested pid doesn't exist\n" );
	return NULL;

}

