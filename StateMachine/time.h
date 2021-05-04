#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wiringPi.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <errno.h>




#define IDLE 0
#define SLEEP 1
//#define WAKE_INIT 2
//#define WAKE_BEGIN 3
#define WAKING 2
#define FORCE_WAKE 3
#define NORMAL_WAKE 4
#define MANUAL_LIGHT 5
//#define FULL_BRIGHTNESS 8
#define ON 1
#define OFF 0
#define DONE 0
#define LIGHT 1
#define DEEP 2

#define PWM_PIN 1


int time_transfer(int hour, int min);


int Parse_InputTime();

int Parse_Alarm();

int Parse_Manual();

int Parse_sys_time(char *time);


int Parse_State(char *time);


int Parse_Brightness(char *time);