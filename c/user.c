/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

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
	syskillTest = syskill( MAX_PROC + 1 , 5 );			// (1/2)
	kprintf( "syskill test result returns %d, expecting -512\n", syskillTest );
	
	syswaitTest = syswait( MAX_PROC + 1 );				// (2/2)
	kprintf( "syswait test result returns %d, expecting -1\n", syswaitTest );

	sysstop();
}

void     root( void ) {
/****************************/

	char input[256];
	int *inputData = &input[0];

	char pass[256];
	int *userPass = &pass[0];

	char user[6] = { 'c', 's', '4', '1', '5', '\0' };

	char password[16] = { 'E', 'v', 'e', 'r', 'y',
						  'o', 'n', 'e',
						  'g', 'e', 't', 's',
						  'a', 'n',
						  'A' };
	Bool auth = TRUE;

	char userCommand[256];

	char ps[2] = { 'p', 's' };
	char ex[2] = { 'e', 'x' };
	char EOF = ;
	char K = 'k';
	char A = 'a';
	char T = 't';

/*
	1. Prints a banner that says Welcome to Xeros - an experimental OS
	2. Opens the keyboard.
	3. Prints Username:
	4. Reads the username - the only username you need to support is cs415
	5. Turns keyboard echoing off
	6. Prints Password:
	7. Reads the password
	8. Closes the keyboard
	9. Verifies the username and password
	10. If the verification fails goes back to step 1 (The password is to be EveryonegetsanA)
	11. Create the shell program.
	12. Wait for the shell program to exit
	13. Go back to step 1
*/
	do {
		do {
			kprintf( "\n\nWelcome to Xeros - an experimental OS" );

			sysopen( 1 );

			kprintf( "\nUsername: " );

			sysread( 1, inputData, sizeof(char[6]) );
	
			sysioctl( 1, 55 );
			kprintf( "\nPassword: " );

			sysread( 1, userPass, sizeof(char[16]) );
	
			sysclose( 1 );

			for( int i = 0, i < 6, i++ ) {
				if( input[i] != user[i] ) {
					auth = FALSE;
					break;
				}
			}

			for( int i = 0, i < 16, i++ ) {
				if( pass[i] != password[i] ) {
					auth = FALSE;
					break;
				}
			}

		} while( !auth );

/*
	NOTE: code from here on is the shell program

	1. Print the prompt >
	2. Reads the command - each command ends when the enter key is pressed.
	3. The first word on the line is the command.
	4. If the command does not exist print “Command not found” and go to step 1
	5. If the command exists then create the process corresponding to that command and remember
	   the process ID until the process exits.
	6. If the command line ended with “ &” got back to 1.
	7. Otherwise wait for the command to finish with syswait().
*/

		sysopen( 1 );

		kprintf( "\n\n> " );
		sysioctl( 1, 53, 0xa );

		

	} while(TRUE);
	
	


		
	

    sysstop();
    
    for( ;; ) {
     sysyield();
    }
    
}

