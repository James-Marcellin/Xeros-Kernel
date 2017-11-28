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

//	1. showing prioritization and signal intterupting each other
	
//	2. syssighandler() test

//	3. syskill() test
	syskillTest = syskill( 0, 2525 );
	kprintf( "syskill test result returns %d, expecting -561\n", syskillTest );

//	4. syswait() test
	syswaitTest = syswait( 0 );
	kprintf( "trying to terminate idle process\n" );
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

	sysioctlTest = sysioctl( 0, 56 );	// 9. test for uncovered cases (1/2)
	kprintf( "sysioctl test result returns %d, expecting 0\n", sysioctlTest );

//	8. sysread() when there are more characters buffered in kernal than read request
	sysreadTest = sysread( 0, buffer, sizeof(int) - 1 );
	kprintf( "sysread test result returns %d, expecting ??\n", sysreadTest );

	syscloseTest = sysclose( 0 );	// 9. tests for uncovered cases (2/2)
	kprintf( "sysclose test result returns %d, expecting 0\n", syscloseTest );

	sysstop();
}

void     root( void ) {
/****************************/

    sysstop();
    
    for( ;; ) {
     sysyield();
    }
    
}

