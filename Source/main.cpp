/*********************************************************************
* Author: William McMahan
* Date: 5/16/2010
* DataLogger program is based upon starter code NI-DAQ "ContAcq-IntClk.c"
*
* Notes from the starter code.
**********************************************************************
* ANSI C Example program:
*    ContAcq-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a continuous amount of
*    data using the DAQ device's internal clock.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage range.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the rate of the acquisition. Also set the Samples per
*       Channel control. This will determine how many samples are
*       read at a time. This also determines how many points are
*       plotted on the graph each time.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the rate for the sample clock. Additionally, define the
*       sample mode to be continuous.
*    4. Call the Start function to start the acquistion.
*    5. Read the data in the EveryNCallback function until the stop
*       button is pressed or an error occurs.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

#include <time.h>
#include <stdio.h>
#include <conio.h>		// Needed for _kbhit()
#include <windows.h>
#include <string>
#include "fileHelper.h"
#include "nidaq.h"


int main(void)
{		
	//// Setup the file variables
	std::string filename = "test.dat";

	//if(fileExists(filename)) {
	//	
	//	printf("File %s already exists!\n", filename.c_str());
	//	
	//	if( !permissionToOverwrite(filename) ) {
	//		printf("End of program, press Enter key to quit\n");
	//		getchar();
	//		return 1;
	//	}

	//}

	//// Open the file for writing.
	FILE* dataFile;
	dataFile = fopen( filename.c_str(), "wb");
	//// Write a header for the file.
	///* TO BE DONE */
		int writeStatus = 0;
		
	/* Write a header for the file. */
	writeStatus = fprintf(dataFile, "FILE_NAME:\t%s\n",filename.c_str() );	
	// Write a time stamp.
	time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
	writeStatus = fprintf(dataFile, "TIME_STAMP:\t%s",asctime( localtime(&ltime) ) );
	writeStatus = fprintf(dataFile, "ACC_SENSOR:\tADXL322, 5V, ?Hz\n");
	writeStatus = fprintf(dataFile, "ACC_UNIT:\tV\n");
	writeStatus = fprintf(dataFile, "SAMPLE_RATE:\t%i\n", SAMPLE_RATE);
	writeStatus = fprintf(dataFile, "DATA_FORMAT:\tFLOAT\n");
	writeStatus = fprintf(dataFile, "DATA: [accMLx, accMLy, accMLz, accMRx, accMRy, accMRz]\n");
	if (writeStatus < 0) {
		printf("** Error writing header data to file. Is the file open? ***\n");
		return 1;
	}

	nidaq ni(dataFile);
	ni.start();

	// Close out the file.
	fclose(dataFile);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}