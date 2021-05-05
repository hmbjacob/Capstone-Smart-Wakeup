
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wiringPi.h>
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
    int Systime = 256;

    // Initialize PWM and pins
    wiringPiSetup () ;
    pinMode(PWM_PIN, PWM_OUTPUT);
    pwmWrite(PWM_PIN, 0);
    
    // initialize getting data from output file of sleeping analyse algorithm.
    FILE *fp;
    char *line;
    size_t bufsize = 32;
    ssize_t read;
    fp = fopen("/logs/logfile.txt","r");
    line = (char *) malloc (bufsize + 1);
    // open file to write
    FILE *fw;
    fw = fopen( "state_output.txt" , "w" );
    fclose(fw);
    char feedback_str[80];

    // Return error if file failed to open
    if(fp==NULL){
        printf("\nError: Can't find file\n");
        exit(0);
    }


    // Main polling loop
    while (1) {
        // open file to append data to the output file
        char inputString[62] = "";
        fw = fopen( "state_output.txt" , "a" );
        //==================
        // NOTE: These two variables need to be modified in the future
        //       The first test set these variables from terminal input 
        //       In the future, they will come from file io and system clock.
        //       BPM comes from the ECG.
        //==================

        delay(500);
        
        if(Systime >= 1440)
            Systime = 120;
        else
            Systime += 5;
            
        //int Systime = Parse_sys_time(__TIME__);
        
        
        // variables for alarm control
        bool Alarm = false;
        int Alarm_Start_time=0;
        bool alarm_start = false; 
        bool Sync_Data = false;
        // Start_time: record when the start sequence start.
        // These values are initialized in the WAKE_INIT State
        int Start_time;
        int alarm_option = 0;

        // Parse Input parameters
        Input_time = Parse_InputTime();
        alarm_option = Parse_Alarm();
        Light_Switch = Parse_Manual();

        // Parse data from Nick's output file
        char input_time[80];
        strcpy(input_time, parse_time_print(Input_time));
        char sys_time[40];
        strcpy(sys_time, parse_time_print(Systime));
        printf("INPUT time: %s, System time: %s\n", input_time, sys_time);
        //printf("INPUT time: %d, system_time: %d\n", Input_time, Systime);
        // get data from the sleeping algorithm
        if((read=getline(&line,&bufsize,fp))!=-1){
            Sleep_State = Parse_State(line);
            brightness  = Parse_Brightness(line); 
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
                    printf("in state WAKING, next state FORCE_WAKE\n");
                // otherwise, stay in WAKING
                } else if(Sleep_State==DONE&&(Systime<Input_time)){
                    state = NORMAL_WAKE;
                    printf("in state WAKING, next state NORMAL_WAKE\n");
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
        

        // Make sure the Light is OFF at the IDLE state
        if(state == IDLE)
            intensity = 0;
        // Make sure the Light is at full brightness at the Manual Light state
        else if(state == MANUAL_LIGHT)
            intensity = 500;
        else
        // Modify brightness according to the output from Nick's Algorithm 
            intensity = brightness*5;
        
        
            
        // Write value to PWM
        pwmWrite(PWM_PIN, intensity);
        
        // Turn on the alarm if necessary

        if(Alarm == true){
            Alarm_Start_time = Systime;
            Alarm = false;
            alarm_start = true;
            // Turn on the alarm here
        }

        if(Systime>=Alarm_Start_time+1&&alarm_start){
            // Turn off the alarm here
        }


        // ================================================
        // The following section output the state and systime
        // to the output file to transmit to Phone APP
        // ================================================
        char feedback_light[19];
        
        //NOT SLEEP
        if(Sleep_State == 0 && Sleep_State != DONE){
            char feedback_wake[18];
            sprintf(feedback_wake,"WAKE;%s\n\0",sys_time);
                fputs(feedback_wake, fw);
        }
        //LIGHT
        else if(Sleep_State == 1 && Sleep_State != DONE){
            
            sprintf(feedback_light,"LIGHT;%s\n\0",sys_time);
            //for(int i=0; i<sizeof(feedback_light); i++){
                fputs(feedback_light, fw);
            //}
        }
        //DEEP
        else if (Sleep_State == 2 && Sleep_State != DONE){
            char feedback_deep[18];
            sprintf(feedback_deep,"DEEP;%s\n\0",sys_time);
            //for(int i=0; i<sizeof(feedback_deep); i++){
                fputs(feedback_deep, fw);
            //}
        }
        // Close file after write one line
       fclose(fw);
       
        


    }
    // close and free pointers
    free(line);
    fclose(fp);
    fclose(fw);
    
    return 1;
}
