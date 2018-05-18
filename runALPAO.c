/*

To compile:
gcc runALPAO.c -o runALPAO -L/home/kvangorkom/milk/lib -I/home/kvangorkom/milk/src/ImageStreamIO -limagestreamio -lasdk

Usage:
./runALPAO  <serialnumber> <normalize_bool> <bias_bool>

What it does:
Connects to the ALPAO DM (indicated by its serial number), initializes the
shared memory image (if it doesn't already exist), and then commands the DM
from the image when the associated semaphores post.

To do:
-Input arguments: DM serial number (becomes SMimage name?)
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

// interrupt signal handling for safe DM shutdown
volatile sig_atomic_t stop;

void handle_signal(int signal)
{
    if (signal == SIGINT)
    {
        printf("Stopping the ALPAO control loop.\n");
        stop = 1;
    }
}

// intialize DM and shared memory and enter DM command loop
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
    Scalar *   dminputs;

    //initialize DM
    asdkDM * dm = NULL;
    dm = asdkInit(serial);

    // Get number of actuators
    ret = asdkGet( dm, "NbOfActuator", &tmp );
    nbAct = (UInt) tmp;

    //------All this should be factored out-----
    // Initialize SMimage if it doesn't already exist
    // or it does but it's shaped incorrectly.
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
      SMimage[0].array.D[i] = 0.;
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
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_handler = handle_signal;
    sigaction(SIGINT, &action, NULL);
    stop = 0;
    while (!stop)
    {
        printf("Waiting on commands.\n");
        // Wait on semaphore update
        ImageStreamIO_semwait(&SMimage[0], 0);
        
        // Send Command to DM
        if (!stop) // Skip DM on interrupt signal
        {
            // Cast to array type ALPAO expects
            dminputs = (Scalar*) calloc( nbAct, sizeof( Scalar ) );
            for ( idx = 0 ; idx < nbAct ; idx++ )
            {
                dminputs[idx] = SMimage[0].array.D[idx];
            }
            printf("Sending command to ALPAO %s.\n", serial);
            ret = sendCommand(dm, dminputs);
        }
    
    }


    // This should maybe delete the shmim too.
    // Otherwise, you can get a backlog next time you start the loop.

    // Safe DM shutdown on interrupt
    printf("Resetting and releasing the ALPAO %s.\n", serial);
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
    //signal(SIGINT, inthand);
    int ret = controlLoop();

    return ret;
}
