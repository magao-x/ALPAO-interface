#include <stddef.h>

/* Alpao SDK C Header */
#include "asdkWrapper.h"

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Get DM serial number from environment variable
char* getSerial()
{
    return "BAX150"; // Hard-coded for now
}

/* Send command to mirror */
int sendCommand()
{
    UInt nbAct, act, idx;
    COMPL_STAT ret;
    Scalar *   data;
    Scalar     tmp;

    asdkDM * dm = NULL;
    char* serial = "";
    
    /* Get serial number */
    serial = getSerial();

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
    
    /* Initialize data */
    data = (Scalar*) calloc( nbAct, sizeof( Scalar ) );
    for ( idx = 0 ; idx < nbAct ; idx++ )
    {
        data[idx] = 0.;
        printf("%f\n", data[idx]);
    }
    data[25] = 0.5;

    /* Send the command to the DM */
    ret = asdkSend( dm, data );

    /* Release memory */
    free( data );

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    int ret = sendCommand();
    
    /* Print last error if any */
    asdkPrintLastError();

    return ret;
}
