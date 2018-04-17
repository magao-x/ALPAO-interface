/* Alpao SDK Legacy Header */
#include "acedev5.h"

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_DM 2

/* Example using legacy code and multiple mirrors */
int legacyExample()
{
    int nbAct = 0, act, idx;
    acecsCOMPL_STAT ret;
    
    const int nDm = N_DM;
    int       dmIds[N_DM];

    double * data;
    int    i_tmp[N_DM];

	char   serial[128] = "";
	
	/* Get serial number */
    printf( "Please enter the S/N within the following format: BXXYYY (see DM backside)\n" );
    scanf( "%s", serial );

    /* Load configuration file */
	/* NOTE: For this example, we simulate the control of two mirrors using the same serial number,
	   you should use one serial number per mirror. To control a single mirror use the following parameters:
			   ret = acedev5Init( 1, dmIds, serial ); */
    ret = acedev5Init( nDm, dmIds, serial, serial );

    if ( ret != acecsSUCCESS )
    {
        return -1;
    }
        
    /* Get the number of actuators */
    ret = acedev5GetNbActuator( nDm, dmIds, i_tmp );
	for ( idx = 0; idx < N_DM; idx++ )
    {
		nbAct += i_tmp[ idx ];
    }

    /* Check errors */
    if ( ret != acecsSUCCESS )
    {
        return -1;
    }
    
    printf( "Total number of actuators: %d\n", nbAct );

    /* Initialize data */
    data = (double*) calloc( nbAct, sizeof( double ) );
    for ( idx = 0 ; idx < nbAct ; idx++ )
    {
        data[idx] = 0;
    }

    /* Send value to the DM */
    printf( "Send data on mirrors (data LED should blink, 10%% at 1Hz):\n" );
    for ( act = 0; act < nbAct && ret == acecsSUCCESS; act++ )
    {
		printf(".");fflush(stdout);

        data[ act ] = 0.10;
        ret = acedev5Send( nDm, dmIds, data );
		sleep( 1 );
        data[ act ] = 0;
    }
	printf( "Done.\n" );

    /* Release memory */
    free( data );

    /* Check errors */
    if ( ret != acecsSUCCESS )
    {
        return -1;
    }

    /* Release */
    acedev5Release( nDm, dmIds );

    return 0;
}

/* Main program */
int main()
{
    int ret = legacyExample();
    
    /* Print last error if any */
    acecsErrDisplay();
    
    return ret;
}
