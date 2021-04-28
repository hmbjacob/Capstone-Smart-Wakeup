/*
* fakedata.c
* Made by Jacob McClellan
* Dream Team 9
* A file to generate fake accelerometry and heart rate Data

* Additional Notes:
    - needs more spikes!!
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void printAccel (FILE* fp, int t, double max); // used to print acceleration data, given time and max value
void printHR (FILE* fp, int t, double min, double max); // used to print HR data, given time and max and min values



int main(void)
{

    srand(time(NULL)); // insert random seed here
    // Preset Data
    int time = 22; // Based on 24-hr clock
    
    // Individual's Makeup
    int RHR = rand() % 35 + 45;        // Resting Heart Rate
    int natCycles = rand() % 2 + 5;    // Number of Sleep cycles
    
    // Create text files to store fake data
    FILE *hp;
    FILE *ap;
    hp = fopen("HR.txt", "w+");  // Heart rate text file
    ap = fopen("ACC.txt", "w+"); // Acceleration text file
    fprintf(hp, "Heart rate data: \n");
    fprintf(ap, "Accelerometer Data: \n");

    int t = 0; // minutes elasped

    // Generate data for before falling asleep

    // Generate data for falling asleep
    
    for (; t < 5 + rand() % 60; t++) {
        printHR (hp, t, RHR-5, RHR+15);// hover around resting while falling asleep
        printAccel(ap, t, 1.5);
    }


    // Now asleep, go through the sleep cycles
    for (int i = 1; i <= natCycles; i++) { // i represents the iteration of the current cycle

        int T_L_NREM = rand() % 10 + 35 - (i-1)*5  + t;          // Light NREM Sleep Duration
        int T_D_NREM = rand() % 10 + 35 - (i-1)*5  + T_L_NREM ;  // Deep NREM Sleep Duration
        int T_REM =    rand() % 10 + 5  + (i-1)*10 + T_D_NREM;   // REM Sleep Duration
        

        // Stage 1 / 2 NREM
        for (; t < T_L_NREM; t++) {
            printHR (hp, t, RHR-8, RHR+8);
            printAccel(ap, t, 1.2);
        }
        // Stage 3 / 4 NREM
        for (; t < T_D_NREM; t++) {
            printHR (hp, t, RHR-9, RHR+5);
            printAccel(ap, t, 1.1);
        }
        // REM Sleep
        for (; t < T_REM; t++) {
            printHR (hp, t, RHR-7, RHR+8);
            printAccel(ap, t, 1);
        }
          
    }
    
    int old_t = t;

    // Final pre-wakeup stage 1
    for (; t < old_t + 10 + rand() % 10; t++) {
        printHR (hp, t, RHR-6, RHR+10);
        printAccel(ap, t,  1.3);
    }
    
    old_t = t;

    // Awake
    for (; t < old_t + 5 + rand() % 15; t++) {
        printHR (hp, t, RHR+5, RHR+20);
        printAccel(ap, t, 1.6);
    }

    
    fprintf(hp, "End of file");
    fprintf(ap, "End of file");

    fclose(hp);
    fclose(ap);
    
    return 0;
}

void printAccel (FILE* fp, int t, double max) {
    int val = (int)((max - .9)*200);
    fprintf(fp, "%d;%f\n", t*60, .9 + (double)(rand() % val)/200);
}

void printHR (FILE* fp, int t, double min, double max) {
    int val = (int)((max - min)*200);
    fprintf(fp, "%d;%d\n", t*60, (int)(min + (double)(rand() % val)/200));
}
