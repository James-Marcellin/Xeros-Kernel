/* kbc.c : Keyboard
 */

#include <xeroskernel.h>
#include <stdarg.h>


short ready = isKeyboardDataReady();
void addCharToBuffer(char character);

int ENTERKEY = 0xa;
dvfunc   deviceTable[4];
unsigned char Pressed;
unsigned char keyboardBuffer[4];
unsigned char eofChar;
unsigned Bool isEchoing;

extern int kbdopen(pcb* process, dvfunc* device, int dvnum) {
    kprintf("kbdopen()\n");
    
    file_descriptor* echoKbd = &process->fileDescriptorTable[KBD_ECHO];
    file_descriptor* nonEchoKbd = &process->fileDescriptorTable[KBD_NON_ECHO];
    
    if (echoKbd->device || nonEchoKbd->device) {
        kprintf("A device is already open. Returning error\n");
        return -1;
    } else {
        kprintf("Opening keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
        isEchoing = (Bool) dvnum;
        enable_irq(1,0);
        process->fileDescriptorTable[dvnum].device = device;
		process->fileDescriptorTable[dvnum].status = DEVICE_OPENED;
    }
    
    return 0;
}

extern int kbdclose(pcb* process, dvfunc* device, int dvnum) {
    kprintf("kbdclose()\n");
    kprintf("Closing keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
    enable_irq(1,1);
    process->fileDescriptorTable[dvnum].device = 0;
	process->fileDescroptorTable[dvnum].status = DEVICE_CLOSED;
    return 0;
}

// writes are not supported by keyboards, return -1 automatically
extern int kbdwrite( void *buff, int bufflen ) {
    return -1;
}

extern int kbdRead(void* buff, unsigned int bufflen) {
	if (isEchoing){
    kprintf("In echo read - %c\n", Pressed);
    memset(buff, Pressed, bufflen);
    return 0;
    }
     
    if(!isEchoing) {
    kprintf("In NonEcho Read\n");
    return 0;
    }
    return -1;
}

/*
extern int kbdNonEchoRead(void* buff, unsigned int bufflen) {
    kprintf("In NonEcho Read\n");
    return 0;
}
*/


extern int kbdioctl(unsigned long command, char newEofChar) {
	if(command == 53){
	eofChar = newEofChar;
	return 0;
	}
	
	if(command == 55}{
	isEchoing = FALSE;
	return 0;
	}
	
	if (command == 56){
	isEchoing = TRUE;	
	return 0;
	}
	return -1;
}

extern int kbd_handler() {
    if(ready) {
        unsigned char fromPort = inb(KBDPORT1);
        unsigned char character = kbtoa(fromPort);
        
        if ((int) character == eofChar) {
            addCharToBuffer(Pressed);
            kprintf("%c", Pressed);
        } else if(character == ENTERKEY) {
            Pressed = '\n';
            addCharToBuffer(Pressed);
            kprintf("%c", Pressed);
        } else if(character) {
            kprintf("%c", character);
            Pressed = character;
            addCharToBuffer(character);
    }
    
    return 0;
}
// insert character into keyboardBuffer[]
void addCharToBuffer(char character) {
    int i;
    for (i = 0; i < 4; i++) {
        if (! keyboardBuffer[i]) {
            keyboardBuffer[i] = character;
        }
    }
}


short isKeyboardDataReady() {
    return (inb(KBDPORT2) & 0x01);
}
