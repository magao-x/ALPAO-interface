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

/* Reset mirror values */
int resetMirror()
{
    COMPL_STAT ret;
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
    ret = asdkReset( dm );

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    int ret = resetMirror();
    
    /* Print last error if any */
    asdkPrintLastError();

    return ret;
}