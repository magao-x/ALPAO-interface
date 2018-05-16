/*
gcc runALPAO.c -o runALPAO -L/home/kvangorkom/milk/lib -I/home/kvangorkom/milk/src/ImageStreamIO -limagestreamio -lasdk



control loop:
-initializes ALPAO
-enters semwait loop
    - when semaphore is posted, apply SMimage to ALPAO
-clean break out of loop (ctrl c signal? does it stop semwait?) that releases ALPAO


To do:
-Input arguments: DM serial number (becomes SMimage name?)
-Figure out data types and shape for DM command
-Figure out how to break out of loop
-Write a few basic shell scripts with semposts: (set pix, set from fits file, etc)
-Write python code to do semposts

To do (future):
-Calibration (bias, displacement normalization)
-Multiplexed virtual DM

*/


/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <signal.h>

/* milk */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()

/* Alpao SDK C Header */
#include "asdkWrapper.h"


volatile sig_atomic_t stop;

void inthand(int signum) {
    stop = 1;
}

int controlLoop()
{
    char * serial = "BAX150"; //Make this an input
    int n, idx;
    UInt nbAct;
    COMPL_STAT ret;
    Scalar     tmp;
    long naxis; // number of axis
    uint8_t atype;     // data type
    uint32_t *imsize;  // image size 
    int shared;        // 1 if image in shared memory
    int NBkw;          // number of keywords supported

    //initialize DM
    asdkDM * dm = NULL;
    dm = asdkInit(serial);

    // Get number of actuators
    ret = asdkGet( dm, "NbOfActuator", &tmp );
    nbAct = (UInt) tmp;

    //------All this should be factored out-----
    // Initialize SMimage if it doesn't already exist
    IMAGE* SMimage;
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));

    naxis = 2;
    imsize = (uint32_t *) malloc(sizeof(uint32_t)*naxis);
    imsize[0] = nbAct;
    imsize[1] = 1;
    
    // image will be float type
    // see file ImageStruct.h for list of supported types
    atype = _DATATYPE_DOUBLE;
    // image will be in shared memory
    shared = 1;
    // allocate space for 10 keywords
    NBkw = 10;
    // create an image in shared memory
    ImageStreamIO_createIm(&SMimage[0], serial, naxis, imsize, atype, shared, NBkw);

    /*
    int i;
    for (i = 0; i < nbAct; i++)
    {
      SMimage[0].array.F[i] = 0.;
    }

    // post all semaphores
    ImageStreamIO_sempost(&SMimage[0], -1);
        
    SMimage[0].md[0].write = 0; // Done writing data
    SMimage[0].md[0].cnt0++;
    SMimage[0].md[0].cnt1++;
    */
    //--------------------------------

    //initialize read
    ImageStreamIO_read_sharedmem_image_toIMAGE(serial, &SMimage[0]);


    // Check SMimage dimensionality and size against DM
    if (SMimage[0].md[0].naxis != 2) {
        printf("SM image naxis = %d\n", SMimage[0].md[0].naxis);
        return -1;
    }
    if (SMimage[0].md[0].size[0] != nbAct) {
        printf("SM image size (axis 1) = %d", SMimage[0].md[0].size[0]);
        return -1;
    }

    // control loop
    //n = 100;
    //for ( idx = 0 ; idx < n ; idx++ )
    stop = 0;
    while (!stop)
    {
        printf("I'm looping!\n");
        // Wait on semaphore update
        ImageStreamIO_semwait(&SMimage[0], 0);

        // Send Command to DM
        ret = sendCommand(dm, &SMimage[0].array.F); //This doesn't work!
    }
    printf("Done!\n");

    // Reset and release ALPAO
    asdkReset(dm);
    ret = asdkRelease(dm);
    dm = NULL;

    return 0;
}

/* Send command to mirror */
int sendCommand(asdkDM * dm, Scalar * data)
{
    COMPL_STAT ret;

    /* Send the command to the DM */
    ret = asdkSend(dm, data);

    /* Release memory */
    free( data );

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    signal(SIGINT, inthand);
    int ret = controlLoop();

    return ret;
}
