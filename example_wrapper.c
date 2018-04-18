/* Alpao SDK C Header */
#include "asdkWrapper.h"

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* Wait for human action */
void pause()
{
    printf( "Press ENTER to exit... \n" );
    fflush( stdin ); getchar();
}

/* Example using C API */
int wrapperExample()
{
    UInt nbAct, act, idx;
    COMPL_STAT ret;
    Scalar *   data;
    Scalar     tmp;

    asdkDM * dm = NULL;
    
	char   serial[128] = "";
	
	/* Get serial number */
    printf( "Please enter the S/N within the following format: BXXYYY (see DM backside)\n" );
    scanf_s( "%s", serial, sizeof(serial) );

    /* Load configuration file */
    dm = asdkInit( serial );
    if ( dm == NULL )
    {
        return -1;
    }
        
    /* Get the number of actuators */
    ret = asdkGet( dm, "NbOfActuator", &tmp );
    nbAct = (UInt) tmp;

    /* Check errors */
    if ( ret != SUCCESS )
    {
        return -1;
    }
    
    printf( "Number of actuators: %d\n", nbAct );

    /* Initialize data */
    data = (Scalar*) calloc( nbAct, sizeof( Scalar ) );
    for ( idx = 0 ; idx < nbAct ; idx++ )
    {
        data[idx] = 0;
    }

    /* Send value to the DM */
    printf( "Send data on mirror (data LED should blink):\n" );
    for ( act = 0; act < nbAct && ret == SUCCESS; act++ )
    {
		printf( "." );

        data[ act ] = 0.10;
        ret = asdkSend( dm, data );
		Sleep( 200 );
        data[ act ] = 0;
    }
	printf( "Done.\n" );

    /* Release memory */
    free( data );

    /* Reset mirror values */
    asdkReset( dm );

    /* Release */
    asdkRelease( dm );
    dm = NULL;

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    int ret = wrapperExample();
    
    /* Print last error if any */
    asdkPrintLastError();

    pause();
    return ret;
}