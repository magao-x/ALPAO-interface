/*
To compile:
>>>gcc runALPAO.c -o build/runALPAO -L$HOME/milk/lib -I$HOME/milk/src/ImageStreamIO -limagestreamio -lasdk
(You must already have the ALPAO SDK and milk installed.)

Usage:
To run with defaults
>>>./runALPAO <serialnumber>
To run with bias and normalization conventions disabled (not yet implemented):
>>>./runALPAO <serialnumber> --nobias --nonorm

For help:
>>>./runALPAO --help

What it does:
Connects to the ALPAO DM (indicated by its serial number), initializes the
shared memory image (if it doesn't already exist), and then commands the DM
from the image when the associated semaphores post.

Still to be implemented or determined:
-Bias and displacement (though placeholder functions exist)
-Mapping from normalized ASDK inputs (-1 -> +1) to physical displacement
-Multiplexed virtual DM
*/

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <signal.h>
#include <argp.h>

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
        printf("\nExiting the ALPAO control loop.\n");
        stop = 1;
    }
}

// Initialize the shared memory image
void initializeSharedMemory(char * serial, UInt nbAct)
{
    long naxis; // number of axis
    uint8_t atype;     // data type
    uint32_t *imsize;  // image size 
    int shared;        // 1 if image in shared memory
    int NBkw;          // number of keywords supported
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

    /* flush all semaphores to avoid commanding the DM from a 
    backlog in shared memory */
    ImageStreamIO_semflush(&SMimage[0], -1);
    
    // write 0s to the image
    imarray[0].md[0].write = 1; // set this flag to 1 when writing data
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
}

// intialize DM and shared memory and enter DM command loop
int controlLoop(char * serial, int nobias, int nonorm)
{
    int n, idx;
    UInt nbAct;
    COMPL_STAT ret;
    Scalar     tmp;
    Scalar *   dminputs;
    IMAGE* SMimage;

    //initialize DM
    asdkDM * dm = NULL;
    dm = asdkInit(serial);
    if (dm == NULL)
    {
        return -1;
    }

    // Get number of actuators
    ret = asdkGet( dm, "NbOfActuator", &tmp );
    if (ret == -1)
    {
        return -1;
    }
    nbAct = (UInt) tmp;

    // initialize shared memory image to 0s
    initializeSharedMemory(serial, nbAct);

    // connect to shared memory image (SMimage)
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));
    ImageStreamIO_read_sharedmem_image_toIMAGE(serial, &SMimage[0]);

    // Validate SMimage dimensionality and size against DM
    if (SMimage[0].md[0].naxis != 2) {
        printf("SM image naxis = %d\n", SMimage[0].md[0].naxis);
        return -1;
    }
    if (SMimage[0].md[0].size[0] != nbAct) {
        printf("SM image size (axis 1) = %d", SMimage[0].md[0].size[0]);
        return -1;
    }

    // set DM to all-0 state to begin
    printf("ALPAO %s: initializing all actuators to 0 displacement.\n", serial);
    ImageStreamIO_semwait(&SMimage[0], 0);

    // SIGINT handling
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_handler = handle_signal;
    sigaction(SIGINT, &action, NULL);
    stop = 0;

    // control loop
    while (!stop)
    {
        printf("ALPAO %s: waiting on commands.\n", serial);
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
            printf("ALPAO %s: sending command with nobias=%d and nonorm=%d.\n", serial, nobias, nonorm);
            ret = sendCommand(dm, dminputs);
            if (ret == -1)
            {
                return -1;
            }
        }
    }

    // Safe DM shutdown on interrupt
    printf("ALPAO %s: resetting and releasing DM.\n", serial);
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

    return ret;
}

/* Placeholder for DC bias */
void bias_inputs(Scalar * dm_inputs)
{
    // do something
}

/* Placeholder for normalization 

This will require knowledge of the ALPAO
influence functions. Should this reference
an external calibration file?*/
void normalize_inputs(Scalar * dm_inputs)
{
    // do something
}


/*
Argument parsing
*/

/* Program documentation. */
static char doc[] =
  "runALPAO-- enter the ALPAO DM command loop and wait for milk shared memory images to be posted at <serial>";

/* A description of the arguments we accept. */
static char args_doc[] = "serial";

/* The options we understand. */
static struct argp_option options[] = {
  {"nobias",  'b', 0,      0,  "Disable automatically biasing the DM (enabled by default)" },
  {"nonorm",    'n', 0,      0,  "Disable displacement normalization (enabled by default)" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *args[1];                /* serial */
  int nobias, nonorm;
};

/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'b':
      arguments->nobias = 1;
      break;
    case 'n':
      arguments->nonorm = 1;
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

/* Main program */
int main( int argc, char ** argv )
{
    struct arguments arguments;

    /* Default values. */
    arguments.nobias = 0;
    arguments.nonorm = 0;

    /* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    // enter the control loop
    int ret = controlLoop(arguments.args[0], arguments.nobias, arguments.nonorm);
    asdkPrintLastError();

    return ret;
}
