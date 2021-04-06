/*
* fakedata.c
* Made by Jacob McClellan
* Dream Team 9
* A file to generate fake accelerometry and heart rate Data

* Additional Notes:
    - Can only set going to sleep time to whole hour, probably doesnt matter
    - minute format could be fixed
    - A lot of use of random within like-stages. data points may benefit from further interconnection
    - Accel data currently unitless and pretty arbitrary. Needs experimental data to help
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>



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

    // Generate data for before going to bed
    int t_BB = 3 + rand() % 6; // a little bit of time out of bed
    int r_elv = rand() % 15 + 5; // random elevation while moving
    for (;t < t_BB; t++) {
        fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 10 + r_elv); // generate HR data for pre-bed
        fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, 2 + rand() % 15); // generate ACC data for pre-bed
    }


    // Generate data for falling asleep
    for (; t < 5 + rand() % 30; t++) {
        fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 10 - 5); // hover around resting while falling asleep
        fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, 1 + rand() % 7);
    }

    // Now asleep, go through the sleep cycles
    for (int i = 1; i <= natCycles; i++) { // i represents the iteration of the current cycle

        int T_L_NREM = rand() % 10 + 35 - (i-1)*5  + t;          // Light NREM Sleep Duration
        int T_D_NREM = rand() % 10 + 35 - (i-1)*5  + T_L_NREM ;  // Deep NREM Sleep Duration
        int T_REM =    rand() % 10 + 5  + (i-1)*10 + T_D_NREM;   // REM Sleep Duration
        

        // Stage 1 / 2 NREM
        for (; t < T_L_NREM; t++) {
            fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 5 - 15); // HR
            fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, rand() % 4); // ACC
        }
        // Stage 3 / 4 NREM
        for (; t < T_D_NREM; t++) {
            fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 5 - 15); // HR
            fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, rand() % 2); // ACC
        }
        // REM Sleep
        for (; t < T_REM; t++) {
            fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 10 - 10); // HR
            fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, rand() % 2); // ACC
        }
          
    }

    // Final pre-wakeup stage 1
    for (; t < 5 + rand() % 10; t++) {
        fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 10 - 5); // HR
        fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, 2 + rand() % 4); // ACC
    }

    // Awake
    for (; t < 2 + rand() % 15; t++) {
        fprintf(hp, "%d:%d - HR: %d\n", (time + (t/60))%24, t%60, RHR + rand() % 10 + 5); // slightly elavted from baseline
        fprintf(ap, "%d:%d - ACC: %d\n", (time + (t/60))%24, t%60, 5 + rand() % 5); // moving a lot more now awake
    }

    
    fprintf(hp, "End of file");
    fprintf(ap, "End of file");

    fclose(hp);
    fclose(ap);
    
    return 0;
}