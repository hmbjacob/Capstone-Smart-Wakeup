
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define IDLE 0
#define SLEEP 1
#define WAKE_INIT 2
#define WAKE_BEGIN 3
#define WAKING 4
#define FORCE_WAKE 5
#define NORMAL_WAKE 6
#define MANUAL_LIGHT 7

int main(){


    int state = 0;

    while (1) {
        char inputString[62] = "";

        fgets(inputString, 62, stdin); //get the input & save
        //printf("%s", inputString);

        char *pointerToString = strtok(inputString, " "); //initial split the string

        while (pointerToString != NULL) { //split till the end by while-loop
            pointerToString = strtok(NULL, " ");
        }


        switch (state)
        {

            case IDLE:

                break;

            case SLEEP:
                /* code */
                break;
        
            case WAKE_INIT:
                /* code */
                break;
        
            case WAKE_BEGIN:
                /* code */
                break;
        
            case WAKING:
                /* code */
                break;
        
            case FORCE_WAKE:
                /* code */
                break;
        
            case NORMAL_WAKE:
                /* code */
                break;

            case MANUAL_LIGHT:
                /* code */
                break;
        
        
        
        default:
            break;
        }


    }


    return 1;
}