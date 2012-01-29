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

#include <stdio.h>
#include <conio.h>		// Needed for _kbhit()
#include <windows.h>
#include <string>
#include "helperNIDAQ.h"
#include "fileHelper.h"


int main(void)
{		
	//// Setup the file variables
	std::string filename = "test.txt";

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

	// Variables for NI Error Handling
	int32 error=0;
	
	// TaskHandle for analog input
	TaskHandle  taskHandleForAnalogGather = 0;

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/

	// Start continuous
	gotoDAQmxErrChk (startContinuousAnalogRecording(taskHandleForAnalogGather, dataFile));
	
Error:
	char errBuff[2048]={'\0'};
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandleForAnalogGather!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandleForAnalogGather);
		DAQmxClearTask(taskHandleForAnalogGather);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	// Try to Zero out the analog output
	TaskHandle  taskHandleForZeroAnalogOut = 0;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	printf("\n***Zeroing the Analog Output Channel.\n");
	gotoDAQmxErrChk2 (DAQmxCreateTask("",&taskHandleForZeroAnalogOut));
	gotoDAQmxErrChk2 (DAQmxCreateAOVoltageChan(taskHandleForZeroAnalogOut,"Dev1/ao3","Zero",0.0,5.0,DAQmx_Val_Volts,NULL));
	gotoDAQmxErrChk2 (DAQmxCfgSampClkTiming(taskHandleForZeroAnalogOut,"",1000,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,5));

	float64 zeroData[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
	int32 tmp = 0;
	gotoDAQmxErrChk2 ( DAQmxWriteAnalogF64(taskHandleForZeroAnalogOut,5,0,10.0,DAQmx_Val_GroupByChannel,zeroData,&tmp,NULL) );
	
	gotoDAQmxErrChk2 (DAQmxStartTask(taskHandleForZeroAnalogOut));
	
Error2:
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	DAQmxStopTask(taskHandleForZeroAnalogOut);
	DAQmxClearTask(taskHandleForZeroAnalogOut);

	// Close out the file.
	fclose(dataFile);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}