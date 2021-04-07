
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define IDLE 0
#define SLEEP 1
#define WAKE_INIT 2
#define WAKE_BEGIN 3
#define WAKING 4
#define FORCE_WAKE 5
#define NORMAL_WAKE 6
#define MANUAL_LIGHT 7
#define ON 1
#define OFF 0
#define WAKE 0
#define LIGHT 1
#define DEEP 2
int main(){

    int Sleep_State  = 0; // Sleep_State == 0: Not Sleep State 
                          // Sleep_State == 1: Light Sleep State
                          // Sleep_State == 2: Deep Sleep State 
    int ECG_Status = 1;   // Assume the ECG is on for testing, change this when deal with 
    int state = 0;

    while (1) {
        char inputString[62] = "";
        //==================
        // NOTE: These two variables need to be modified in the future
        //       The first test set these variables from terminal input 
        //       In the future, they will come from file io and system clock.
        //       BPM comes from the ECG.
        //==================

        int Systime = 200;
        int Input_time;
        int BPM;
        int Light_Switch;
        bool Alarm = false;
        bool Sync_Data = false;
        // Start_time: record when the start sequence start.
        // Current_BPM: record the current BPM when the sequence starts
        // N: Count number, start with 1
        // These values are initialized in the WAKE_INIT State
        int Start_time;
        int Current_BPM;
        int N;
        fgets(inputString, 62, stdin); //get the input & save
        //printf("%s", inputString);
        if(inputString=="LIGHT"){
            Sleep_State = LIGHT;
        } else if(inputString=="DEEP"){
            Sleep_State = DEEP;
        } else if(inputString=="WAKE"){
            Sleep_State = WAKE;
        } else if(inputString=="200"){
            Input_time = 200;
        } else if(inputString=="240"){
            Input_time = 240;
        } else if(inputString=="260"){
            Input_time = 260;
        } else if(inputString=="270"){
            Input_time = 270;
        }
        char *pointerToString = strtok(inputString, " "); //initial split the string

        while (pointerToString != NULL) { //split till the end by while-loop
            pointerToString = strtok(NULL, " ");
        }


        switch (state){

            case IDLE:{

                // If the user is wake or ECG is off, stay in the IDLE state
                if(Sleep_State==WAKE || ECG_Status == OFF){
                    printf("in state IDLE, next state IDLE\n");
                    state = IDLE;
                // If the user fall asleep, change the state to SLEEP state
                } else if(Sleep_State==LIGHT || Sleep_State == DEEP){
                    printf("in state IDLE, next state SLEEP\n");
                    state = SLEEP;
                // if the device is set to manual mode, change state to manual state
                } else if(Light_Switch == ON){
                    printf("in state IDLE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // Otherwise, stay in the same state
                } else {
                    printf("in state IDLE, next state IDLE\n");
                    state = IDLE;
                }
                break;
            }

            case SLEEP:{
                if(Sleep_State==WAKE){
                    printf("in state SLEEP, next state IDLE\n");
                    state = IDLE;
                // If the user manually control the light, change to Manual light state
                } else if(Light_Switch==ON){
                    printf("in state SLEEP, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // if the User is still sleep, and the time is not yet one hour before the set time, stay in SLEEP state.
                } else if((Sleep_State==LIGHT || Sleep_State==DEEP)&&(Systime < (Input_time-60))){
                    printf("in state SLEEP, next state SLEEP\n");
                    state = SLEEP;
                // if the User is in Light sleep and there are 30 mins before set time, or
                // if the User is in DEEP sleep and there are 60 mins before set time, set the state to WAKE_INIT.
                } else if(((Sleep_State==DEEP)&&(Systime>Input_time-60)) ||
                          ((Sleep_State==LIGHT)&&(Systime>Input_time-30))){
                    printf("in state SLEEP, next state WAKE_INIT\n");
                    state = WAKE_INIT;
                // Otherwise, keep the state in SLEEP state.
                } else {
                    printf("in state SLEEP, next state SLEEP\n");
                    state = SLEEP;
                }
                break;
            }
        
            case WAKE_INIT:{
                // Initialize variables 
                Start_time = Systime;
                Current_BPM = BPM;
                N = 1;
                // If the user manually control the light, change to manual mode
                if(Light_Switch==ON){
                    printf("in state WAKE_INIT, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // Otherwise, change the state to WAKE_BEGIN to begin wake up the user. 
                } else{
                    printf("in state WAKE_INIT, next state WAKE_BEGIN\n");
                    state = WAKE_BEGIN;
                }
                break;
            }
            
            case WAKE_BEGIN:{
                // If the user manually control the light, change to manual mode
                if(Light_Switch==ON){
                    printf("in state WAKE_BEGIN, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // increament the brightness of the light for 1/10 of total brightness
                } else if(Systime >= (N*(Input_time-Start_time)/10+Start_time)){
                    printf("in state WAKE_BEGIN, next state WAKING\n");
                    state = WAKING;
                // if the time is met the requried increament time, stay in this state.
                } else{
                    printf("in state WAKE_BEGIN, next state WAKE_BEGIN\n");
                    state = WAKE_BEGIN;
                }
                break;
            }
        
            case WAKING:{
                if((Systime<Input_time)&&(Sleep_State==DEEP||Sleep_State==LIGHT)){
                    state = WAKE_BEGIN;
                    printf("in state WAKING, next state WAKE_BEGIN\n");
                } else if(Light_Switch==ON){
                    state = MANUAL_LIGHT; 
                    printf("in state WAKING, next state MANUAL_LIGHT\n");
                } else if((Sleep_State==WAKE)&&(Systime<Input_time)){
                    printf("in state WAKING, next state NORMAL_WAKE\n");
                    state = NORMAL_WAKE;
                } else if((Sleep_State==LIGHT||Sleep_State==DEEP)&&(Systime>=Input_time)){
                    printf("in state WAKING, next state FORCE_WAKE\n");
                    state = FORCE_WAKE;
                } else{
                    printf("in state WAKING, next state WAKING\n");
                    state = WAKING;
                }
                break;
            }
        
            case FORCE_WAKE:{
                if(Light_Switch==ON){
                    printf("in state FORCE_WAKE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                } else{
                    printf("in state FORCE_WAKE, next state IDLE\n");
                    Alarm = true;
                    Sync_Data = true;
                    state = IDLE;
                }
                break;
            }
        
            case NORMAL_WAKE:{
                if(Light_Switch==ON){
                    printf("in state NORMAL_WAKE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                } else {
                    printf("in state NORMAL_WAKE, next state IDLE\n");

                    Sync_Data = true;
                    state = IDLE;
                }
                break;
            }

            case MANUAL_LIGHT:{
                if(Light_Switch == ON){
                    printf("in state MANUAL_LIGHT, next state MANUAL_LIGHT\n");

                    state = MANUAL_LIGHT;
                } else if((Light_Switch==OFF)&&(Sleep_State==LIGHT || Sleep_State==DEEP)){
                    printf("in state MANUAL_LIGHT, next state SLEEP\n");

                    state = SLEEP;
                } else if((Light_Switch==OFF)&&(Sleep_State==WAKE)){
                    printf("in state MANUAL_LIGHT, next state IDLE\n");

                    state = IDLE;
                }
                break;// If the user wake up during the process, change state to IDLE.
            }
        
            default:
                break;
        }


    }


    return 1;
}