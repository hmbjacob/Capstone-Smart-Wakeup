
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <wiringPi.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <errno.h>
#include "time.h"



int main(){

    int Sleep_State  = 0; // Sleep_State == 0: Not Sleep State 
                          // Sleep_State == 1: Light Sleep State
                          // Sleep_State == 2: Deep Sleep State 
    int ECG_Status = 1;   // Assume the ECG is on for testing, change this when deal with 
    int state = 0;
    int alarm = 0;
    int intensity = 0;
    int Input_time;
    int BPM;
    int Light_Switch;
    int brightness = 0;
    int Full_Bright_Time = 10;

    wiringPiSetup () ;
    pinMode(PWM_PIN, PWM_OUTPUT);
    pwmWrite(PWM_PIN, 0);
    
    // initialize getting data from output file of sleeping analyse algorithm.
    FILE *fp;
    char *line;
    size_t bufsize = 32;
    ssize_t read;
    fp = fopen("state_data.txt","r");
    line = (char *) malloc (bufsize + 1);

    if(fp==NULL){
        printf("\nError: Can't find file\n");
        exit(0);
    }

    while (1) {
        char inputString[62] = "";
        //==================
        // NOTE: These two variables need to be modified in the future
        //       The first test set these variables from terminal input 
        //       In the future, they will come from file io and system clock.
        //       BPM comes from the ECG.
        //==================

        int Systime = Parse_sys_time(__TIME__);
        

        bool Alarm = false;
        bool Sync_Data = false;
        // Start_time: record when the start sequence start.
        // Current_BPM: record the current BPM when the sequence starts
        // N: Count number, start with 1
        // These values are initialized in the WAKE_INIT State
        int Start_time;
        int Current_BPM;
        int N;
        int alarm_option = 0;
        Parse_Parameters(Input_time, alarm_option, Light_Switch);

        // get data from the sleeping algorithm
        if((read=getline(&line,&bufsize,fp))!=-1){
            Sleep_State = Parse_State(line);
            brightness  = Parse_Brightness(line);
            // ================================
            // NEED to inplement stall for demo
            // ================================
        }





        fgets(inputString, 62, stdin); //get the input & save
        //printf("%s", inputString);
        if(strcmp(inputString,"LIGHT\n")==0){
            Sleep_State = LIGHT;
        } else if(strcmp(inputString,"DEEP\n")==0){
            Sleep_State = DEEP;
        } else if(strcmp(inputString,"DONE\n")==0){
            Sleep_State = DONE;
        } else if(strcmp(inputString,"ON\n")==0){
            Light_Switch = ON;
        } else if(strcmp(inputString,"OFF\n")==0){
            Light_Switch = OFF;
        } else{
            Systime = atoi(inputString);
        }

        
        //printf("INPUT TIME: %d\n", Input_time);

        char *pointerToString = strtok(inputString, " "); //initial split the string

        while (pointerToString != NULL) { //split till the end by while-loop
            pointerToString = strtok(NULL, " ");
        }


        switch (state){

            case IDLE:{

                // if the device is set to manual mode, change state to manual state
                if(Light_Switch == ON){
                    printf("in state IDLE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
        
                // If the user is wake and not yet change brightness
                } else if(Sleep_State==DONE && brightness==0){
                    printf("in state IDLE, next state IDLE\n");
                    state = IDLE;

                // If the user fall asleep, change the state to SLEEP state
                } else if(Sleep_State==LIGHT || Sleep_State == DEEP){
                    printf("in state IDLE, next state SLEEP\n");
                    state = SLEEP;
                
                // Otherwise, stay in the same state
                } else {
                    printf("in state IDLE, next state IDLE\n");
                    state = IDLE;
                }
                break;
            }

            case SLEEP:{
                
                // If the user manually control the light, change to Manual light state
                 if(Light_Switch==ON){
                    printf("in state SLEEP, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // if the User is still sleep, and the time is not yet one hour before the set time, stay in SLEEP state.
                } else if((Sleep_State==LIGHT || Sleep_State==DEEP)&&brightness==0){
                    printf("in state SLEEP, next state SLEEP\n");
                    state = SLEEP;
                // if the User is in Light sleep and there are 30 mins before set time, or
                // if the User is in DEEP sleep and there are 60 mins before set time, set the state to WAKE_INIT.
                } else if((Sleep_State==LIGHT || Sleep_State==DEEP)&&brightness>0){
                    printf("in state SLEEP, next state WAKING\n");
                    state = WAKING;
                // Otherwise, keep the state in SLEEP state.
                } else {
                    printf("in state SLEEP, next state SLEEP\n");
                    state = SLEEP;
                }
                break;
            }
        
            
            
            
        
            case WAKING:{
                // If the user manually control the light, change to manual mode
                if(Light_Switch==ON){
                    state = MANUAL_LIGHT; 
                    printf("in state WAKING, next state MANUAL_LIGHT\n");
                // If the User is not yet ready to be wake by full brightness, Go back to Wake begin to increase time
                } else if(Sleep_State!=DONE){
                    state = WAKING;
                    printf("in state WAKING, next state WAKING\n");
                // If the User is ready to be wake by full brightness, Go back to Wake begin to increase time
                } else if(Sleep_State==DONE&&(Systime>=Input_time)){
                    state = FORCE_WAKE;
                    printf("in state WAKING, next state FULL_BRIGHTNESS\n");
                // otherwise, stay in WAKING
                } else if(Sleep_State==DONE&&(Systime<Input_time)){
                    state = NORMAL_WAKE;
                    printf("in state WAKING, next state FULL_BRIGHTNESS\n");
                // otherwise, stay in WAKING
                } else{
                    printf("in state WAKING, next state WAKING\n");
                    state = WAKING;
                }
                break;
            }
        
            case FORCE_WAKE:{
                // If the user manually control the light, change to manual mode
                if(Light_Switch==ON){
                    printf("in state FORCE_WAKE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // Sync data and go to IDLE, and also force wake the user.
                } else{
                    printf("in state FORCE_WAKE, next state IDLE\n");
                    if(alarm_option){
                       Alarm = true; 
                    }
                    
                    Sync_Data = true;
                    state = IDLE;
                }
                break;
            }
        
            case NORMAL_WAKE:{
                // If the user manually control the light, change to manual mode
                if(Light_Switch==ON){
                    printf("in state NORMAL_WAKE, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // Sync data and go to IDLE
                } else {
                    printf("in state NORMAL_WAKE, next state IDLE\n");

                    Sync_Data = true;
                    state = IDLE;
                }
                break;
            }

            case MANUAL_LIGHT:{
                // If the user manually control the light, change to manual mode
                if(Light_Switch == ON){
                    printf("in state MANUAL_LIGHT, next state MANUAL_LIGHT\n");
                    state = MANUAL_LIGHT;
                // If the user is still sleeping when light switch off, go to sleep state.
                } else if((Light_Switch==OFF)&&(Sleep_State==LIGHT || Sleep_State==DEEP)){
                    printf("in state MANUAL_LIGHT, next state SLEEP\n");
                    state = SLEEP;
                } else if((Light_Switch==OFF)&&(Sleep_State==DONE)){
                    printf("in state MANUAL_LIGHT, next state IDLE\n");

                    state = IDLE;
                } else {
                    state = MANUAL_LIGHT;
                    printf("in state MANUAL_LIGHT, next state MANUAL_LIGHT\n");
                }
                break;// If the user wake up during the process, change state to IDLE.
            }
            
            
            default:
                break;
        }
        
        
        intensity = brightness*5;
        
        
            
            
        pwmWrite(PWM_PIN, intensity);


    }

    free(line);
    fclose(fp);
    return 1;
}
