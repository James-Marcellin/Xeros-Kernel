/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */

#define CREATE_FAILURE -1  /* Process creation failed     */



/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


/* Some constants involved with process creation and managment */
 
   /* Maximum number of processes */      
#define MAX_PROC        64           
   /* Kernel trap number          */
#define KERNEL_INT      80
   /* Interrupt number for the timer */
#define TIMER_INT      (TIMER_IRQ + 32)
   /* Minimum size of a stack when a process is created */
#define PROC_STACK      (4096 * 4)    
              
   /* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10        


/* Constants to track states that a process is in */
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_SLEEP     22
#define STATE_RUNNING   23

/* System call identifiers */
#define SYS_STOP        10
#define SYS_YIELD       11
#define SYS_CREATE      22
#define SYS_TIMER       33
#define SYS_GETPID      144
#define SYS_PUTS        155
#define SYS_SLEEP       166
#define SYS_CPUTIMES    178

/***********************/
/* 2.3 related defines */
/***********************/
#define STATE_WAIT		24

#define SYS_SIGHANDLER	160
#define SYS_SIGRETURN	170
#define SYS_KILL		177
#define SYS_WAIT		180

/***********************/
/* 2.4 related defines */
/***********************/
#define SYS_OPEN		44
#define SYS_CLOSE		55
#define SYS_WRITE		66
#define SYS_READ		77
#define SYS_IOCTL		88

/*******************/
/* keyboard number */
/*******************/
#define KBD_NON_ECHO	0
#define KBD_ECHO		1

/*********************/
/* device status code*/
/*********************/
#define DEVICE_CLOSED	0
#define DEVICE_OPENED	1


/* Structure to track the information associated with a single process */

/* A typedef for the signature of the function passed to syscreate */
typedef void (*funcptr)(void);

typedef struct device_function {
  int			dvnum;
  int			dvminornum;
  char*			dvname;
  int			(*dvinit)( void*, ... );
  int			(*dvopen)( void*, ... );
  int			(*dvclose)( void*, ... );
  int			(*dvread)( void*, ... );
  int			(*dvwrite)( void*, ... );
  int			(*dvctl)( void*, ... );
} dvfunc;

extern dvfunc	deviceTable[2];

typedef struct file_descriptor {
  int			majorNum;
  int			minorNum;
  dvfunc		device;
  char*			name;
  int			status;
} fdt;

typedef struct signalEntry {
  funcptr		handler;
  funcptr		*oldhandler;
} signalEntry;


typedef struct struct_pcb pcb;
struct struct_pcb {
  void			*esp;    /* Pointer to top of saved stack           */
  pcb			*next;   /* Next process in the list, if applicable */
  pcb			*prev;   /* Previous proccess in list, if applicable*/
  int			state;  /* State the process is in, see above      */
  unsigned int	pid;    /* The process's ID                        */
  int			ret;    /* Return value of system call             */
                       /* if process interrupted because of system*/
                       /* call                                    */
  signalEntry	signalTable[32];
  int			signalsWaiting;
  long			args;   
  unsigned int	otherpid;
  void			*buffer;
  int			bufferlen;
  int			sleepdiff;
  long			cpuTime;  /* CPU time consumed                     */
  fdt			fileDescriptorTable[4];		   
};

typedef struct signal_stack {
  unsigned int	ret;
  funcptr		handler;
  unsigned int	esp;
  unsigned int	old_sp;
  int			ignoreSignalMask;
}signal_stack;

typedef struct struct_ps processStatuses;
struct struct_ps {
  int  entries;            // Last entry used in the table
  int  pid[MAX_PROC];      // The process ID
  int  status[MAX_PROC];   // The process status
  long cpuTime[MAX_PROC]; // CPU time used in milliseconds
};


/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
  unsigned long        edi;
  unsigned long        esi;
  unsigned long        ebp;
  unsigned long        esp;
  unsigned long        ebx;
  unsigned long        edx;
  unsigned long        ecx;
  unsigned long        eax;
  unsigned long        iret_eip;
  unsigned long        iret_cs;
  unsigned long        eflags;
  unsigned long        stackSlots[];
} context_frame;


/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

int      kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( size_t );


/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, size_t stack );
void     set_evec(unsigned int xnum, unsigned long handler);
void     printCF (void * stack);  /* print the call frame */
int      syscall(int call, ...);  /* Used in the system call stub */
void     sleep(pcb *, unsigned int);
void     removeFromSleep(pcb * p);
void     tick( void );
int      getCPUtimes(pcb *p, processStatuses *ps);

/* Function prototypes for system calls as called by the application */
int          syscreate( funcptr fp, size_t stack );
void         sysyield( void );
void         sysstop( void );
unsigned int sysgetpid( void );
unsigned int syssleep(unsigned int);
void         sysputs(char *str);
int          sysgetcputimes(processStatuses *ps);


/* Keyboard */
unsigned int kbtoa( unsigned char code );

/**************************/
/* 2.3 related prototypes */
/**************************/
int		syssighandler( int signal, void (*newhandler)(void *), void (** oldHandler)(void *) );
void	syssigreturn( void *old_sp );
int		syskill( int PID, int signalNumber );
int		syswait( int PID );

void	sigtramp(void (*handler)(void *), void *cntx);
int		signal(int pid, int sig_num);

pcb		*getProcess( int pid );

/**************************/
/* 2.4 related prototypes */
/**************************/
int		sysopen( int dvnum );
int		sysclose( int fd );
int		syswrite( int fd, void *buff, int bufflen );
int		sysread( int fd, void *buff, int bufflen );
int		sysioctl( int fd, unsigned long command, ... );

void	di_init( void );
int		di_open( pcb* p, int majorNum );
int		di_close( pcb* p, int fd );
int		di_write( pcb* p, int fd, void *buff, int bufflen );
int		di_read( pcb* p, int fd, void *buff, int bufflen );
int		di_ioctl( pcb* p, int fd, unsigned long command, ... );

int 	kbdopen( pcb* process, dvfunc* device, int dvnum );
int 	kbdclose( pcb* process, dvfunc* device, int dvnum );
int 	kbdwrite( void *buff, int bufflen );
int 	kbdRead( void* buff, unsigned int bufflen );
int 	kbdioctl(unsigned long command, char newEofChar);

void 	enable_irq( unsigned int,  int);




/* The initial process that the system creates and schedules */
void     root( void );




void           set_evec(unsigned int xnum, unsigned long handler);


/* Anything you add must be between the #define and this comment */
#endif

