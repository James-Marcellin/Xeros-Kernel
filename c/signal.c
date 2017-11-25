/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */

void sigtramp(void (*handler)(void *), void *cntx){	
	context_frame* cFrame = (context_frame*) cntx;
	handler(cFrame);
	syssigreturn(cFrame);
	}
	
int signal(int pid, int sig_num) {

    if (!isValidSignalNumber(sig_num)) return -2;

	pcb* process = getProcess(pid);	
	if (!process) return -1;
        
    int signalsWaiting = process->signalsWaiting;
    
    signalEntry* sig_entry = &process->signalTable[sig_num];
    funcptr handler = (funcptr)(sig_entry->handler);
    long old_sp = process->esp;
    
    unsigned int stackPosition = process->esp;
    stackPosition -= sizeof(signal_stack);
    signal_stack* signalStack = (signal_stack*)(stackPosition);
    signalStack->handler = handler;
    signalStack->esp = old_sp;
    signalStack->ret = process->ret;
    signalStack->old_sp = old_sp;
    
    stackPos -= sizeof(context_frame);
    context_frame* CF = (context_frame*)(stackPos);
    CF->ebp = stackPos;
    CF->esp = stackPos;
    CF->iret_eip = &sigtramp;
    CF->iret_cs = getCS();
    CF->eflags = 0x00003200;    
        
    process->esp = stackPos;
    
    
    return 0;
	}
