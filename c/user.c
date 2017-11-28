/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void producer( void ) {
/****************************/

    sysstop();
}

void consumer( void ) {
/****************************/

    sysstop();
}

void test( void ) {

	int syssighandlerTest,
		syskillTest,
		syswaitTest,
		sysopenTest,
		syswriteTest,
		sysioctlTest,
		sysreadTest,
		syscloseTest;

	int	myPID = sysgetpid();

//	1. showing prioritization and signal intterupting each other
	
//	2. syssighandler() test

//	3. syskill() test
	syskillTest = syskill( 0, 2525 );
	kprintf( "syskill test result returns %d, expecting -561\n", syskillTest );

//	4. syswait() test
	kprintf( "trying to terminate idle process\n" );
	syswaitTest = syswait( 0 );
	kprintf( "syswait test result returns %d, expecting -1\n", syswaitTest );

//	5. sysopen() test with invalid arguments
	sysopenTest = sysopen( 5 );
	kprintf( "sysopen test result returns %d, expecting -1\n", sysopenTest );

//	6. syswrite() test with invalid file descriptor
	int buffer = 4;
	int *writeTestData = &buffer;

	syswriteTest = syswrite( -1, writeTestData, sizeof(int) );
	kprintf( "syswrite test result returns %d, expecting -1\n", syswriteTest );

//	7. sysioctl() test with invalid commands
	sysopenTest = sysopen( 0 );
	sysioctlTest = sysioctl( 0, 666 );
	kprintf( "sysopen test result returns %d, expecting 1\n", sysopenTest );
	kprintf( "sysioctl test result returns %d, expecting -1\n", sysioctlTest );

//	8. sysread() when there are more characters buffered in kernal than read request
	sysreadTest = sysread( 0, buffer, sizeof(int) - 1 );
	kprintf( "sysread test result returns %d, expecting ??\n", sysreadTest );

//	9. two test cases for scenarios not covered here or in the test program
	syskillTest = syskill( 65, 5 );			// (1/2)
	kprintf( "syskill test result returns %d, expecting -512\n", syskillTest );
	
	kprintf( "trying to kill self\n" );		// (2/2)
	syswaitTest = syswait( myPID );
	kprintf( "syswait test result returns %d, expecting -2\n", syswaitTest );

	sysstop();
}

void     root( void ) {
/****************************/

    sysstop();
    
    for( ;; ) {
     sysyield();
    }
    
}

