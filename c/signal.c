/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>

/* Your code goes here */

void sigtramp( void (*handler)(void *), void *cntx ) {

	context_frame* cFrame = (context_frame*) cntx;
	handler(cFrame);
	syssigreturn(cFrame->esp);

}

// actual signal delivery code
// we don't need to check for the validity of signum anymore because
// we have already done so in the dispatcher
int signal(int pid, int signum) {


	pcb* process = getProcess(pid);
    int signalsWaiting = process->signalsWaiting;
    
    signalEntry* sig_entry = &process->signalTable[signum];
    funcptr handler = (funcptr)(sig_entry->handler);
    long old_sp = process->esp;
    
    unsigned int stackPosition = process->esp;
    stackPosition -= sizeof(signal_stack);
    signal_stack* signalStack = (signal_stack*)(stackPosition);
    signalStack->handler = handler;
    signalStack->esp = old_sp;
    signalStack->ret = process->ret;
    signalStack->old_sp = old_sp;
    
    stackPosition -= sizeof(context_frame);
    context_frame* CF = (context_frame*)(stackPosition);
    CF->ebp = stackPosition;
    CF->esp = stackPosition;
    CF->iret_eip = &sigtramp;
    CF->iret_cs = getCS();
    CF->eflags = 0x00003200;    
        
    process->esp = stackPosition;
        
    return 0;

}
