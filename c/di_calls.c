/* di_calls.c : Device Independant Calls
 */

#include <xeroskernel.h>
#include <stdarg.h>

Bool invalidFd( int fd );

dvfunc*		kbdNonEcho;
dvfunc*		kbdEcho;

// initializer of di_calls
// we only have to deal with 2 devices, an echoing keyboard and a non-echoing keyboard
// the initializer will register the devices into the device table
// and install the corresponding device function codes
void di_init() {

	kbdNonEcho          = &deviceTable[KBD_NON_ECHO];
	kbdNonEcho->dvopen  = (funcptr) kbdopen;
	kbdNonEcho->dvclose = (funcptr) kbdclose;
	kbdNonEcho->dvwrite = (funcptr) kbdwrite;
	kbdNonEcho->dvread  = (funcptr) kbdRead;
	kbdNonEcho->dvctl   = (funcptr) kbdioctl;

	kbdEcho          = &deviceTable[KBD_ECHO];
	kbdEcho->dvopen  = (funcptr) kbdopen;
	kbdEcho->dvclose = (funcptr) kbdclose;
	kbdEcho->dvwrite = (funcptr) kbdwrite;
	kbdEcho->dvread  = (funcptr) kbdRead;
	kbdEcho->dvctl   = (funcptr) kbdioctl;

}

int di_open( pcb* p, int majorNum ) {

	dvfunc* device = &deviceTable[majorNum];
	int dvnum = p->fileDescriptorTable[majorNum].majorNum;

	return device->dvopen( p, device, dvnum );

}

/*	di_calls beyond this point follows the implementation pattern:
		- check if fd is valid and if the corresponding device is opened
		- using fd from the process to determines the index	of the device in the device table
		  then from there determines the function to call
		- calls the function
		- determine the meaning of the return value of the DII call.
*/

int	di_close( pcb* p, int fd ) {

	if( invalidFd( fd ) ) {
		kprintf( "invalid fd\n" );
	} else if( p->fileDescriptorTable[fd].status != DEVICE_OPENED ) {
		kprintf( "targeted device isn't open" );
	} else {

		dvfunc* device = &p->fileDescriptorTable[fd].device;
		int dvnum = p->fileDescriptorTable[fd].majorNum;

		return device->dvclose( p, device, dvnum );

	}

	return -1;

}

int	di_write( pcb* p, int fd, void *buff, int bufflen ) {

	if( invalidFd( fd ) ) {
		kprintf( "invalid fd\n" );
	} else if( p->fileDescriptorTable[fd].status != DEVICE_OPENED ) {
		kprintf( "targeted device isn't open" );
	} else {
		return deviceTable[fd].dvwrite( buff, bufflen );
	}

	return -1;

}

int	di_read( pcb* p, int fd, void *buff, int bufflen ) {

	if( invalidFd( fd ) ) {
		kprintf( "invalid fd\n" );
	} else if( p->fileDescriptorTable[fd].status != DEVICE_OPENED ) {
		kprintf( "targeted device isn't open" );
	} else {
		return deviceTable[fd].dvread( buff, bufflen );
	}

	return -1;

}

int	di_ioctl( pcb* p, int fd, unsigned long command, ... ) {

	if( invalidFd( fd ) ) {
		kprintf( "invalid fd\n" );
	} else if( p->fileDescriptorTable[fd].status != DEVICE_OPENED ) {
		kprintf( "targeted device isn't open" );
	} else {
		return deviceTable[fd].dvctl( command );
	}

	return -1;

}

// Helper to validate passed fd
Bool invalidFd( int fd ) {
	if( fd < 0 || fd > 3 ){
		return TRUE;
	} else {
		return FALSE;
	}
}
