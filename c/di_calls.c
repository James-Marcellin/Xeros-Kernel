/* di_calls.c : DII calls
 */

#include <xeroskernel.h>
#include <stdarg.h>

Bool invalidFd( int fd );

dvfunc		kbdNonEcho;
dvfunc		kdbEcho;

void di_init() {

	kbdNonEcho          = deviceTable[KBD_NON_ECHO];
	kbdNonEcho->dvopen  = (funcptr) kbdopen;
	kbdNonEcho->dvclose = (funcptr) kbdclose;
	kbdNonEcho->dvwrite = (funcptr) kbdwrite;
	kbdNonEcho->dvread  = (funcptr) kbdNonEchoRead;
	kbdNonEcho->dvctl   = (funcptr) kbdioctl;

	kbdEcho          = deviceTable[KBD_ECHO];
	kbdEcho->dvopen  = (funcptr) kbdopen;
	kbdEcho->dvclose = (funcptr) kbdclose;
	kbdEcho->dvwrite = (funcptr) kbdwrite;
	kbdEcho->dvread  = (funcptr) kbdEchoRead;
	kbdEcho->dvctl   = (funcptr) kbdioctl;

}

int di_open( pcb* p, int dvnum ) {

	dvfunc device = deviceTable[majorNum];
	int dvnum = p->fileDescriptorTable[dvnum].majorNum;

	return device->dvopen( p, device, dvnum );

}

int	di_close( pcb* p, int fd ) {

	if( invalidFd( fd ) ) {
		return -1;
	} else {

		dvfunc device = p->fileDescriptorTable[fd].device;
		int dvnum = p->fileDescriptorTable[fd].majorNum;

		return device->dvclose( p, device, dvnum );

	}

}

int	di_write( int fd, void *buff, int bufflen ) {

	if( invalidFd( fd ) ) {
		return -1;
	} else {
		return deviceTable[fd].dvwrite( buff, bufflen );
	}

}

int	di_read( int fd, void *buff, int bufflen ) {

	if( invalidFd( fd ) ) {
		return -1;
	} else {
		return deviceTable[fd].dvread( buff, bufflen );
	}

}

int	di_ioctl( int fd, unsigned long command, ... ) {

	if( invalidFd( fd ) ) {
		return -1;
	} else {
		return deviceTable[fd].dvctl( command );
	}

}

// Helper to validate passed fd
Bool invalidFd( int fd ) {
	if( fd < 0 || fd > 3 ){
		return TRUE;
	} else {
		return FALSE;
	}
}
