// Alpao SDK Header: All types and class are in the ACS namespace
#include "asdkDM.h"
using namespace acs;

// System Headers
#include <iostream>
#include <unistd.h>
using namespace std;

// Example using C++ API
int dmExample()
{
	String serial;

	// Get serial number
    cout << "Please enter the S/N within the following format: BXXYYY (see DM backside)" << endl;
	cin >> serial;
    cin.ignore( 10, '\n' );

    // Load configuration file
    DM dm( serial.c_str() );

    // Get the number of actuators
    UInt nbAct = (UInt) dm.Get( "NbOfActuator" );

    // Check errors
    if ( !dm.Check() )
    {
        return -1;
    }
    
    cout << "Number of actuators: " << nbAct << endl;

    // Initialize data
    Scalar *data = new Scalar[nbAct];
    for ( UInt i = 0 ; i < nbAct ; i++ )
    {
        data[i] = 0;
    }
	
    cout << "Send data on mirrors (data LED should blink, 10% at 1Hz): " << endl;
    // Send value to the DM
    for ( UInt act = 0; act < nbAct && dm.Check(); act++ )
    {
		cout << "." << std::flush;

        data[ act ] = 0.12;
        dm.Send( data );
		sleep( 1 );
        data[ act ] = 0;
    }
    cout << "Done." << endl;
    
    // Release memory
    delete [] data;

    return 0;
}

// Main program
int main( int, char ** )
{
    int ret = dmExample();
    
    // Print last errors if any
    while ( DM::PrintLastError() );

    return ret;
}
