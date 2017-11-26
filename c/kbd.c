/* kbc.c : Keyboard
 */

#include <xeroskernel.h>
#include <stdarg.h>
#include <kbd.h>


void addCharToBuffer(char character);

short isKeyboardDataReady();
int ENTERKEY = 0xa;
dvfunc   deviceTable[4];
unsigned char Pressed;
unsigned char keyboardBuffer[4];
unsigned char eofChar;
Bool isEchoing;





extern int kbdopen(pcb* process, dvfunc* device, int dvnum) {
    kprintf("kbdopen()\n");
    
    fdt* echoKbd = &process->fileDescriptorTable[KBD_ECHO];
    fdt* nonEchoKbd = &process->fileDescriptorTable[KBD_NON_ECHO];
    
    if (echoKbd->status || nonEchoKbd->status) {
        kprintf("A device is already open. Returning error\n");
        return -1;
    } else {
        kprintf("Opening keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
        isEchoing = (Bool) dvnum;
        enable_irq(1,0);
		process->fileDescriptorTable[dvnum].status = DEVICE_OPENED;
    }
    
    return 0;
}

extern int kbdclose(pcb* process, dvfunc* device, int dvnum) {
    kprintf("kbdclose()\n");
    kprintf("Closing keyboard %d (0 = nonecho, 1 = echo)\n", dvnum);
    enable_irq(1,1);
	process->fileDescriptorTable[dvnum].status = DEVICE_CLOSED;
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
	
	if(command == 55){
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
    if(isKeyboardDataReady()) {
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


short isKeyboardDataReady( void ) {
    return (inb(KBDPORT2) & 0x01);
}

static int extchar( unsigned char code){
        state &= ~EXTENDED;
}

unsigned int kbtoa( unsigned char code ){
  unsigned int    ch;
  
  if (state & EXTENDED)
    return extchar(code);
  if (code & KEY_UP) {
    switch (code & 0x7f) {
    case LSHIFT:
    case RSHIFT:
      state &= ~INSHIFT;
      break;
    case CAPSL:
      printf("Capslock off detected\n");
      state &= ~CAPSLOCK;
      break;
    case LCTL:
      state &= ~INCTL;
      break;
    case LMETA:
      state &= ~INMETA;
      break;
    }
    
    return NOCHAR;
  }
  
  
  /* check for special keys */
  switch (code) {
  case LSHIFT:
  case RSHIFT:
    state |= INSHIFT;
    printf("shift detected!\n");
    return NOCHAR;
  case CAPSL:
    state |= CAPSLOCK;
    printf("Capslock ON detected!\n");
    return NOCHAR;
  case LCTL:
    state |= INCTL;
    return NOCHAR;
  case LMETA:
    state |= INMETA;
    return NOCHAR;
  case EXTESC:
    state |= EXTENDED;
    return NOCHAR;
  }
  
  ch = NOCHAR;
  
  if (code < sizeof(kbcode)){
    if ( state & CAPSLOCK )
      ch = kbshift[code];
	  else
	    ch = kbcode[code];
  }
  if (state & INSHIFT) {
    if (code >= sizeof(kbshift))
      return NOCHAR;
    if ( state & CAPSLOCK )
      ch = kbcode[code];
    else
      ch = kbshift[code];
  }
  if (state & INCTL) {
    if (code >= sizeof(kbctl))
      return NOCHAR;
    ch = kbctl[code];
  }
  if (state & INMETA)
    ch += 0x80;
  return ch;
}
