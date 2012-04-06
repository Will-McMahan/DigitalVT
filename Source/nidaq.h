#pragma once

#include <NIDAQmx.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "performanceCounter.h"
#include "DigitalFilter.h"
#include <conio.h>		// Needed for _kbhit()

using namespace std;

// User set parameters
#define MIN_VOLT_IN		 -5
#define MAX_VOLT_IN		 5

#define MIN_VOLT_OUT	 -5
#define MAX_VOLT_OUT	  5

#define SAMPLE_RATE				4000
#define SAMPLES_PER_CHANNEL		100

#define NUM_INPUT_CHANNELS		6

class nidaq
{

public:
	//nidaq(void);
	nidaq(FILE*);
	~nidaq(void);

	void start();

	FILE* file_handle;

private:
	// Handles for input and output tasks.
	TaskHandle h_input;
	TaskHandle h_output;
	
	// Error handling variables
	bool error_status;
	int error_check( int32 error_code);

	// Analog Input/Output parameters
	int sample_rate;
	int samples_per_channel;
	int num_input_channels;

	void cleanup();

	DigitalFilter *hpf_acc1;
	DigitalFilter *hpf_acc2;

};
