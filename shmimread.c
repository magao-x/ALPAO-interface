/*
gcc shmimread.c -o shmimread -L/home/kvangorkom/milk/lib -I/home/kvangorkom/milk/src/ImageStreamIO -limagestreamio
*/


/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

/* milk */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()


/* Read in a test shared memory image */
int shmimread()
{
    char* filename = "test";
    int n, idx;

    IMAGE* SMimage;
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));

    ImageStreamIO_read_sharedmem_image_toIMAGE(filename, &SMimage[0]);

    n = SMimage[0].md[0].size[0] * SMimage[0].md[0].size[1];
    printf("%d\n", n);
    //for ( idx = 0 ; idx < n ; idx++ )
    //{
    //    printf("%f\n", SMimage[0].array.F[idx]);
    //}
    printf("(0,0): %f\n", SMimage[0].array.F[0]);
    //printf("Done\n");

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    int ret = shmimread();

    return ret;
}
