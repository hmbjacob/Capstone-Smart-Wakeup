// Provide feedback input to the State Machine from an existing dataset and current inputs
// Just a * very * simple skeleton to generate ideas

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct State_Manager {
    double brightness;
    int do_wakeup;
} typedef struct State_Manager* S_Man;

// Brief list of prototype methods
S_Man create_manager();				// Constructor
int get_wkup_status(S_Man obj);			// Light, Deep or Wake
double get_intensity(S_Man obj);		// [0, 1] inclusive
int update(uint8_t* data, int data_len);	// Parse and update dataset, return a status code