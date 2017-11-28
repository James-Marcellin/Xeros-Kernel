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

	int sysopenTest,
		syswriteTest,
		sysioctlTest,
		sysreadTest,
		syscloseTest;

//	1. showing prioritization and signal intterupting each other
	
//	2. syssighandler() test

//	3. syskill() test

//	4. syswait() test

//	5. sysopen() test with invalid arguments
	sysopenTest = sysopen( 5 );
	kprintf( "sysopen test result returns %d, expecting -1\n", sysopenTest );

//	6. syswrite() test with invalid file descriptor
	int buffer = 4;
	int *writeTestData = &buffer;

	syswriteTest = syswrite( -1, writeTestData, sizeof(int) );
	kprintf( "syswrite test result returns %d, expecting -1\n", syswriteTest );

//	7. sysioctl() test with invalid commands
	sysopenTest = sysopen( 1 );		// 9. tests for uncovered cases (1/2)
	sysioctlTest = sysioctl( 1, 66 );
	kprintf( "sysopen test result returns %d, expecting 1\n", sysopenTest );
	kprintf( "sysioctl test result returns %d, expecting -1\n", sysioctlTest );

//	8. sysread() when there are more characters buffered in kernal than read request
	sysreadTest = sysread( 1, buffer, sizeof(int) - 1 );
	kprintf( "sysread test result returns %d, expecting ??\n", sysreadTest );

	syscloseTest = sysclose( 1 );	// 9. tests for uncovered cases (2/2)
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

