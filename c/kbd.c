/* kbc.c : Keyboard
 */

#include <xeroskernel.h>
#include <stdarg.h>


short ready = isKeyboardDataReady();
void addCharToBuffer(char character);

int CTL_D = 0x4;
int ENTERKEY = 0xa;
devsw   deviceTable[4];
unsigned char Pressed;
unsigned char keyboardBuffer[4];
unsigned char eofChar;

extern int kbdopen(pcb* process, dvfunc* device, int dvnum) {
    kprintf("kbdopen()\n");
    
    file_descriptor* echoKbd = &process->fileDescriptorTable[KBD_ECHO];
    file_descriptor* nonEchoKbd = &process->fileDescriptorTable[KBD_NONECHO];
    
    if (echoKbd->device || nonEchoKbd->device) {
        kprintf("A device is already open. Returning error\n");
        return -1;
    } else {
        kprintf("Opening keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
        enable_irq(1,0);
        process->fileDescriptorTable[dvnum].device = device;
		process->fileDescriptorTable[dvnum].status = DEVICE_OPENED;
    }
    
    return 0;
}

extern int kbdclose(pcb* process, devsw* device, int dvnum) {
    kprintf("kbdclose()\n");
    kprintf("Closing keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
    enable_irq(1,1);
    process->fileDescriptorTable[dvnum].device = 0;
	process->fileDescroptorTable[dvnum].status = DEVICE_CLOSED;
    return 0;
}

// writes are not supported by keyboards, return -1 automatically
extern int kbdwrite() {
    return -1;
}

extern int kbdEchoRead(void* buff, unsigned int bufflen) {
    kprintf("In echo read - %c\n", Pressed);
    memset(buff, Pressed, bufflen);
    return 0;
}

extern int kbdNonEchoRead(void* buff, unsigned int bufflen) {
    kprintf("In NonEcho Read\n");
    return 0;
}



extern void kbdioctl(char newEofChar) {
    eofChar = newEofChar;
}

extern int kbd_handler() {
    if(ready) {
        unsigned char fromPort = inb(0x60);
        unsigned char character = kbtoa(fromPort);
        
        if ((int)character == CTL_D) {
            kprintf("BREAK!!\n");
        } else if ((int) character == ENTERKEY) {
            Pressed = '\n';
            addCharToBuffer(Pressed);
            kprintf("%c", Pressed);
        } else if(character) {
            kprintf("%c", character);
            Pressed = character;
            addCharToBuffer(character);
        }
    }
    
    return 0;
}

void addCharToBuffer(char character) {
    int i;
    for (i = 0; i < 4; i++) {
        if (! keyboardBuffer[i]) {
            keyboardBuffer[i] = character;
        }
    }
}



short isKeyboardDataReady() {
    return (inb(0x64) & 0x01);
}
