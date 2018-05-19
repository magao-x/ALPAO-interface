# ALPAO-interface
Interface to ALPAO DM-97

To compile with the ASDK and milk [ImageStreamIO](https://github.com/milk-org/ImageStreamIO) libraries:
	
	gcc runALPAO.c -o build/runALPAO -L$HOME/milk/lib -I$HOME/milk/src/ImageStreamIO -limagestreamio -lasdk

To enter the DM control loop with default settings:

	./runALPAO <serialnumber>

`ctrl+c` will exit the loop and safely reset and release the DM. To run with bias and normalization conventions disabled (not yet implemented):

	./runALPAO <serialnumber> --nobias --nonorm

For help:

	./runALPAO --help

Once the control loop is running, the DM can be commanded by writing a 97x1 image to shared memory using your tool of choice. A few examples using the milk package are provided:

	./loadfits <path-to-fits-file> <serial>
	./setpix <value> <actuator-number> <serial>
	
Alternatively, [PyImageStreamIO](https://github.com/milk-org/pyImageStreamIO) provides a simple interface in Python.
