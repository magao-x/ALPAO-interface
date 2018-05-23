/*
To compile:
>>>gcc queryALPAO.c -o build/queryALPAO -L$HOME/milk/lib -I$HOME/milk/src/ImageStreamIO -limagestreamio
(You must already have milk installed.)

To call:
>>>build/./queryALPAO <serial>

Prints out the value of each actuator input from the shared
memory image corresponding to an ALPAO DM, which should
directly correspond to the real actuator stroke state if
the DM is in use.
*/

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

/* milk */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()

/* Reset and Release */
int query_shared_memory(char * serial)
{
	IMAGE * SMimage;
	int idx;
	int nbAct;

	// connect to shared memory image
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));
    ImageStreamIO_read_sharedmem_image_toIMAGE(serial, &SMimage[0]);

    // print out stroke of each actuator
    nbAct = SMimage[0].md[0].size[0];
    for ( idx = 0 ; idx < nbAct ; idx++)
    {
        printf("%s actuator #%d: %lf\n", serial, idx+1, SMimage[0].array.D[idx]);
    }

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    char * serial;

    if (argc < 2)
    {
        printf("Serial number must be supplied.\n");
        return -1;
    }
    serial = argv[1];

    int ret = query_shared_memory(serial);
    return ret;
}