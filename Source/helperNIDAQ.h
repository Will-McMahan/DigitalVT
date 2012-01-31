#ifndef _HELPERNIDAQ_H_
#define _HELPERNIDAQ_H_

#include <NIDAQmx.h>
#include "performanceCounter.h"

// User set parameters
#define MIN_VOLT_ACC		 -5
#define MAX_VOLT_ACC		  5

#define MIN_VOLT_CURRENT	 -5
#define MAX_VOLT_CURRENT	  5

#define SAMPLE_RATE				2500
#define SAMPLES_PER_CHANNEL		50
#define NUM_CHANNELS			6

// Error check macros
#define gotoDAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
#define gotoDAQmxErrChk2(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error2; else
#define returnDAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) return error; else

TaskHandle  h_output;

// Callback Function Prototypes
//int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
//int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int32  totalNumRead=0;
	static int32  totalNumWrote=0;
	float acc1_in[3];
	float acc2_in[3];
	double acc_out[2*SAMPLES_PER_CHANNEL];
	
	int32       numSamplesReadPerChannel=0;
	float64     data[NUM_CHANNELS * SAMPLES_PER_CHANNEL];
	//std::vector<float64> data(NUM_CHANNELS * SAMPLES_PER_CHANNEL);
		
	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	//printf("Started Callback.\n");
	
	gotoDAQmxErrChk (DAQmxReadAnalogF64(taskHandle, SAMPLES_PER_CHANNEL, 0.0, DAQmx_Val_GroupByScanNumber, data, NUM_CHANNELS * SAMPLES_PER_CHANNEL, &numSamplesReadPerChannel,NULL));
	if( numSamplesReadPerChannel>0 ) {

		for (int i = 0; i < SAMPLES_PER_CHANNEL*NUM_CHANNELS; i+=NUM_CHANNELS)
		{
			acc1_in[0] = (float)data[i];
			acc1_in[1] = (float)data[i+1];
			acc1_in[2] = (float)data[i+2];
			
			acc2_in[0] = (float)data[i+3];
			acc2_in[1] = (float)data[i+4];
			acc2_in[2] = (float)data[i+5];
			
			//acc_out[i/NUM_CHANNELS] = data[i] + data[i+1] + data[i+2];								
			//acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] = data[i+3] + data[i+4] + data[i+5];
			acc_out[i/NUM_CHANNELS] = data[i];								
			acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] = data[i+3];

			if (acc_out[i/NUM_CHANNELS] > 5.0) 
				acc_out[i/NUM_CHANNELS] = 5.0;
			else if (acc_out[i/NUM_CHANNELS] < -5.0) 
				acc_out[i/NUM_CHANNELS] = -5.0;

			if (acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] > 5.0) 
				acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] = 5.0;
			else if (acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] < -5.0) 
				acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] = -5.0;

				fwrite(acc1_in, sizeof(float), 3, (FILE*)callbackData);
				fwrite(acc2_in, sizeof(float), 3, (FILE*)callbackData);
		}
		gotoDAQmxErrChk( DAQmxWriteAnalogF64(h_output, SAMPLES_PER_CHANNEL, true, 0.1, DAQmx_Val_GroupByChannel, acc_out, &totalNumWrote, NULL) );       
		//gotoDAQmxErrChk( DAQmxStartTask(h_output));
		printf("Acquired %d samples. Total %d\r",numSamplesReadPerChannel, totalNumRead+=numSamplesReadPerChannel);
		fflush(stdout);
		
	}

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(h_output);
		DAQmxClearTask(h_output);
		
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("%i, DAQmx Error in Callback: %s\n",totalNumWrote,errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	gotoDAQmxErrChk (status);
	
Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);		
		printf("DAQmx Error in Done: %s\n",errBuff);
		DAQmxClearTask(taskHandle);
	}
	return 0;
}

int32 startContinuousAnalogRecording(TaskHandle& inputTaskHandle, FILE* dataFile) {
	// Variables for NI Error Handling
	int32       error=0;
	float64		zeros[SAMPLES_PER_CHANNEL*2] = {0.0};

	//for(int i = 0; i < SAMPLES_PER_CHANNEL*2; i++)
		//zeros[i] = 0;

	
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	returnDAQmxErrChk (DAQmxCreateTask("OUT1",&h_output));
	returnDAQmxErrChk (DAQmxCreateAOVoltageChan(h_output,"Dev1/ao0, Dev1/ao1","",MIN_VOLT_CURRENT,MAX_VOLT_CURRENT,DAQmx_Val_Volts,NULL));
	//returnDAQmxErrChk (DAQmxCreateAOVoltageChan(h_output,"Dev1/ao1","",MIN_VOLT_CURRENT,MAX_VOLT_CURRENT,DAQmx_Val_Volts,NULL));
	//returnDAQmxErrChk ( DAQmxSetAODataXferMech(h_output, "Dev1/ao0, Dev1/ao1", DAQmx_Val_DMA) );
	returnDAQmxErrChk ( DAQmxSetBufOutputBufSize(h_output, SAMPLES_PER_CHANNEL*2) );
	returnDAQmxErrChk (DAQmxCfgSampClkTiming(h_output,"OnboardClock",SAMPLE_RATE,DAQmx_Val_Rising,DAQmx_Val_ContSamps,SAMPLES_PER_CHANNEL));	


	returnDAQmxErrChk( DAQmxWriteAnalogF64(h_output, SAMPLES_PER_CHANNEL, true, 1.0, DAQmx_Val_GroupByChannel, zeros, NULL, NULL) );

	
	//	returnDAQmxErrChk( DAQmxWaitUntilTaskDone(h_output, 10.0) );

	//returnDAQmxErrChk (DAQmxCreateTask("OUT2",&h_output2));
	//returnDAQmxErrChk (DAQmxCreateAOVoltageChan(h_output2,"Dev1/ao1","",MIN_VOLT_CURRENT,MAX_VOLT_CURRENT,DAQmx_Val_Volts,NULL));
	//returnDAQmxErrChk (DAQmxCfgSampClkTiming(h_output2,"OnboardClock",SAMPLE_RATE,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,SAMPLES_PER_CHANNEL));	

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	//returnDAQmxErrChk (DAQmxStartTask(h_output1));
	//returnDAQmxErrChk (DAQmxStartTask(h_output2));

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	returnDAQmxErrChk (DAQmxCreateTask("GetAnalog",&inputTaskHandle));

	//acc1_in
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai0","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai7","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai8","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	//returnDAQmxErrChk (DAQmxCreateAOVoltageChan(inputTaskHandle,"Dev1/ao0","",MIN_VOLT_CURRENT,MAX_VOLT_CURRENT,DAQmx_Val_Volts,NULL));

	//acc2_in
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai2","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai3","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	returnDAQmxErrChk (DAQmxCreateAIVoltageChan(inputTaskHandle,"Dev1/ai4","",DAQmx_Val_RSE,MIN_VOLT_ACC,MAX_VOLT_ACC,DAQmx_Val_Volts,NULL));
	
	// Set up timing.
	returnDAQmxErrChk (DAQmxCfgSampClkTiming(inputTaskHandle, "OnboardClock", SAMPLE_RATE, DAQmx_Val_Rising, DAQmx_Val_ContSamps, SAMPLES_PER_CHANNEL));
	returnDAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(inputTaskHandle, DAQmx_Val_Acquired_Into_Buffer, SAMPLES_PER_CHANNEL, 0, EveryNCallback, dataFile));
	
	// Registers a callback function to receive an event when a task stops due to an error or when a finite acquisition task or finite generation 
	// task completes execution. A Done event does not occur when a task is stopped explicitly, such as by calling DAQmxStopTask.
	returnDAQmxErrChk (DAQmxRegisterDoneEvent(inputTaskHandle,0,DoneCallback,NULL));



/*------------------------------------------------------*/
	// DAQmx Start Task
	returnDAQmxErrChk (DAQmxStartTask(inputTaskHandle));
	
	PerformanceCounter timer;
	timer.StartCounter();

	/*********************************************/
	// DAQmx Wait Code
	/*********************************************/
	
	// Delay stopping the Task until the 'ENTER' key has been pressed.
	printf("Acquiring samples continuously. Press Enter to interrupt\n");
	while(1) {
		if(_kbhit()) {	// Check the keypress buffer
			char keycheck = getch();
			if(keycheck == 13) // ENTER key
				break;
			//if(keycheck == 'e') // The following is an unsupported property for the NI USB-6259, so it flags causes an error.  For debugging purposes.
			//	returnDAQmxErrChk ( DAQmxSetAOIdleOutputBehavior(h_output1, "Dev1/a03", DAQmx_Val_ZeroVolts ));
			else
				printf("You Pressed: ASCII %d. Please press ENTER key to quit.\n", keycheck);
		}
	}
	printf("\n*** Detected ENTER key press, quiting... \n");	
	
	//int waitTime = (int)(1000.0/SAMPLE_RATE * SAMPLES_PER_CHANNEL);
	//printf("Detected ENTER Press. Waiting for the end of the current acquisition... %d\n", waitTime);
	//Sleep(waitTime);

	timer.StopCounter();
	double time = timer.GetElapsedTime();

	returnDAQmxErrChk (DAQmxStopTask(inputTaskHandle));
	returnDAQmxErrChk (DAQmxClearTask(inputTaskHandle));

	returnDAQmxErrChk (DAQmxStopTask(h_output));
	returnDAQmxErrChk (DAQmxClearTask(h_output));

	printf("\n***Acquired data for approximately %1.3f seconds.\n", time);
	printf("Expect approximately  %1.f samples.\n", time * SAMPLE_RATE);

	return 0;
}

#endif
