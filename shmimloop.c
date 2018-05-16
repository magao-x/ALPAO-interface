/*
gcc shmimloop.c -o shmimloop -L/home/kvangorkom/milk/lib -I/home/kvangorkom/milk/src/ImageStreamIO -limagestreamio
*/


/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

/* milk */
#include "ImageStruct.h"   // cacao data structure definition
#include "ImageStreamIO.h" // function ImageStreamIO_read_sharedmem_image_toIMAGE()


/* Loop that waits for shared memory
images to post and then does something.

To post in milk:
> readshmim "test"
> setpix "test" 1 250 250
*/
int shmimloop()
{
    char* filename = "test";
    int n, idx;

    IMAGE* SMimage;
    SMimage = (IMAGE*) malloc(sizeof(IMAGE));
    ImageStreamIO_read_sharedmem_image_toIMAGE(filename, &SMimage[0]);

    n = 100;
    for ( idx = 0 ; idx < n ; idx++ )
    {
        printf("%d\n", idx);
        ImageStreamIO_semwait(&SMimage[0], 0);
    }
    printf("Done\n");

    return 0;
}

/* Main program */
int main( int argc, char ** argv )
{
    int ret = shmimloop();

    return ret;
}
