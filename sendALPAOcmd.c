/*
gcc sendALPAOcmd.c -o sendALPAOcmd -L/home/kvangorkom/milk/lib -I/home/kvangorkom/milk/src/ImageStreamIO -lasdk -limagestreamio
*/


/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

/* Alpao SDK C Header */
#include "asdkWrapper.h"

/* milk */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()
//#include "COREMOD_iofits.h"

//#include <fitsio.h>

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

    /* Read fits file */
    //long test;
    //test = load_fits("/home/kvangorkom/irisao_measured_flats.fits", "test", 2);

    //fitsfile *fptr;   /* FITS file pointer, defined in fitsio.h */
    //int status = 0;   /* CFITSIO status value MUST be initialized to zero! */
    //fits_open_file(&fptr, "/home/kvangorkom/test.fits", READONLY, &status);
    //fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
    //              buffer, &anynull, &status);
    
    /* Initialize data */
    data = (Scalar*) calloc( nbAct, sizeof( Scalar ) );
    //for ( idx = 0 ; idx < nbAct ; idx++ )
    //{
    //    data[idx] = 0.;
    //    printf("%f\n", data[idx]);
    //}
    //data[25] = 0.5;

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
