#include "nidaq.h"

nidaq::nidaq(FILE* fhandle)
{
	fileHandle = fhandle;
	
	cout << "Initializing the NIDAQ." << endl;

	// Initialize variables
	h_input = 0;
	h_output = 0;
	error_status = false;

	////TODO: Parameterize these in the future.
	//sample_rate = SAMPLE_RATE;
	//samples_per_channel = SAMPLES_PER_CHANNEL;
	//num_input_channels = NUM_INPUT_CHANNELS;

	/*************************************************/
	/* DAQmx Configure Analog Input */
	/*************************************************/
	error_check (DAQmxCreateTask("INPUT",&h_input));

	//acc1_in
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai0","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai7","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai8","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));

	//acc2_in
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai2","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai3","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));
	error_check (DAQmxCreateAIVoltageChan(h_input,"Dev1/ai4","",DAQmx_Val_RSE,MIN_VOLT_IN,MAX_VOLT_IN,DAQmx_Val_Volts,NULL));
	
	// Set up timing.
	error_check (DAQmxCfgSampClkTiming(h_input, "OnboardClock", SAMPLE_RATE, DAQmx_Val_Rising, DAQmx_Val_ContSamps, SAMPLES_PER_CHANNEL));
	
	//error_check( DAQmxStartTask(h_input) );
	
	/*************************************************/
	// DAQmx Configure Analog Output
	/*************************************************/
	error_check (DAQmxCreateTask("OUT",&h_output));
	error_check (DAQmxCreateAOVoltageChan(h_output,"Dev1/ao0, Dev1/ao1","",MIN_VOLT_OUT,MAX_VOLT_OUT,DAQmx_Val_Volts,NULL));
	error_check (DAQmxCfgSampClkTiming(h_output,"OnboardClock",SAMPLE_RATE,DAQmx_Val_Rising,DAQmx_Val_ContSamps,SAMPLES_PER_CHANNEL));	
	//error_check (DAQmxCfgDigEdgeStartTrig(h_output,"/Dev1/ai/StartTrigger",DAQmx_Val_Rising));
	error_check (DAQmxSetWriteRegenMode(h_output, DAQmx_Val_DoNotAllowRegen));

	//float64		zeros[SAMPLES_PER_CHANNEL*2] = {0.0};
	//error_check( DAQmxWriteAnalogF64(h_output, SAMPLES_PER_CHANNEL, true, 1.0, DAQmx_Val_GroupByChannel, zeros, NULL, NULL) );		
	//error_check( DAQmxStartTask(h_output) );
		
	//TODO: Write function to close up all the handles and whatnot if there is an error.
	if (error_status)
	{
		cout << "ERROR During Initialization!" << endl;
		cleanup();				
	}
}

nidaq::~nidaq(void)
{
	cleanup();
}

void nidaq::cleanup()
{
	//Zero the Analog output
	printf("\n***Zeroing the Analog Output Channel.\n");
	float64		zeros[SAMPLES_PER_CHANNEL*2] = {0.0};
	error_check( DAQmxStopTask(h_output));
	error_check( DAQmxWriteAnalogF64(h_output, SAMPLES_PER_CHANNEL, true, 1.0, DAQmx_Val_GroupByChannel, zeros, NULL, NULL) );

	if(h_output)
	{
		cout << "Clearing h_output." << endl;
		error_check( DAQmxStopTask(h_output) );
		error_check( DAQmxClearTask(h_output) );
	}
	if(h_input)
	{
		cout << "Clearing h_input." << endl;
		error_check( DAQmxStopTask(h_input) );
		error_check( DAQmxClearTask(h_input) );
	}
}

int nidaq::error_check(int32 error_code)
{
	//TODO: Throw an exception?
	char    errBuff[2048]={'\0'};

	if( DAQmxFailed(error_code) )
	{
		error_status = true;
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		//cout << "DAQmx Error: ";
		//cout << errBuff << endl;
		printf("DAQmx Error: %s\n",errBuff);
		return 1;
	}
	
	return 0;
}

void nidaq::start()
{
	bool32 isLate = false;

	static int32  totalNumRead=0;
	static int32  totalNumWrote=0;
	float acc1_in[3];
	float acc2_in[3];
	double acc_out[2*SAMPLES_PER_CHANNEL];
	
	int32       numSamplesReadPerChannel=0;
	float64     data[NUM_INPUT_CHANNELS * SAMPLES_PER_CHANNEL];

	PerformanceCounter timer, timer2;
	double time2;
	timer.StartCounter();
	
	cout << "Started Loop..." << endl;
	while(!error_status)
	{					
		//cout << "In Loop! " << endl;

		if(error_status)
			break;
		if(_kbhit()) {	// Check the keypress buffer
			char keycheck = getch();
			if(keycheck == 13) // ENTER key
				break;
			//if(keycheck == 'e') // The following is an unsupported property for the NI USB-6259, so it flags causes an error.  For debugging purposes.
			//	returnDAQmxErrChk ( DAQmxSetAOIdleOutputBehavior(h_output1, "Dev1/a03", DAQmx_Val_ZeroVolts ));
			else
				printf("You Pressed: ASCII %d. Please press ENTER key to quit.\n", keycheck);
		}

		error_check( DAQmxReadAnalogF64(h_input, SAMPLES_PER_CHANNEL, 1.0, DAQmx_Val_GroupByScanNumber, data, NUM_INPUT_CHANNELS * SAMPLES_PER_CHANNEL, &numSamplesReadPerChannel, NULL) );
		
		//timer2.StopCounter();
		//time2= timer2.GetElapsedTime();
		//cout << "Read! " << time2;
		//timer2.StartCounter();

		for (int i = 0; i < SAMPLES_PER_CHANNEL*NUM_INPUT_CHANNELS; i+=NUM_INPUT_CHANNELS)
		{
			acc1_in[0] = (float)data[i];
			acc1_in[1] = (float)data[i+1];
			acc1_in[2] = (float)data[i+2];
			
			acc2_in[0] = (float)data[i+3];
			acc2_in[1] = (float)data[i+4];
			acc2_in[2] = (float)data[i+5];
			
			//acc_out[i/NUM_CHANNELS] = data[i] + data[i+1] + data[i+2];								
			//acc_out[i/NUM_CHANNELS + SAMPLES_PER_CHANNEL] = data[i+3] + data[i+4] + data[i+5];
			acc_out[i/NUM_INPUT_CHANNELS] = data[i];								
			acc_out[i/NUM_INPUT_CHANNELS + SAMPLES_PER_CHANNEL] = data[i+3];

			// Put Rails on the 
			if (acc_out[i/NUM_INPUT_CHANNELS] > 5.0) 
				acc_out[i/NUM_INPUT_CHANNELS] = 5.0;
			else if (acc_out[i/NUM_INPUT_CHANNELS] < -5.0) 
				acc_out[i/NUM_INPUT_CHANNELS] = -5.0;

			if (acc_out[i/NUM_INPUT_CHANNELS + SAMPLES_PER_CHANNEL] > 5.0) 
				acc_out[i/NUM_INPUT_CHANNELS + SAMPLES_PER_CHANNEL] = 5.0;
			else if (acc_out[i/NUM_INPUT_CHANNELS + SAMPLES_PER_CHANNEL] < -5.0) 
				acc_out[i/NUM_INPUT_CHANNELS + SAMPLES_PER_CHANNEL] = -5.0;

			fwrite(acc1_in, sizeof(float), 3, fileHandle);
			fwrite(acc2_in, sizeof(float), 3, fileHandle);
		}
		
		//Sleep(10.0);
		//timer2.StopCounter();
		//time2= timer2.GetElapsedTime();
		//cout << " Process! " << time2;
		//timer2.StartCounter();
		
		//error_check( DAQmxWaitUntilTaskDone(h_output, 1.0) );
		//error_check( DAQmxStopTask(h_output));	// This is an important step.  http://digital.ni.com/public.nsf/allkb/BFCE83133C0ECAD786256E6000814B68
		error_check( DAQmxWriteAnalogF64(h_output, SAMPLES_PER_CHANNEL, true, 0.1, DAQmx_Val_GroupByChannel, acc_out, &totalNumWrote, NULL) );       
		
		timer2.StopCounter();		
		time2= timer2.GetElapsedTime();
		//cout << " Write! " << time2;
		//timer2.StartCounter();
		//cout << " Loop Time: " << std::setprecision(2) << time2;
		timer2.StartCounter();

		printf("Loop Time: %.3f Processed %d samples. Total %d\r",time2, numSamplesReadPerChannel, totalNumRead+=numSamplesReadPerChannel);
		fflush(stdout);
	}	

	timer.StopCounter();
	
	double time = timer.GetElapsedTime();

	printf("\n***Acquired data for approximately %1.3f seconds.\n", time);
	printf("Expect approximately  %1.f samples.\n", time * SAMPLE_RATE);
}
