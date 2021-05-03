#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <wiringPi.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <errno.h>


int time_transfer(int hour, int min);


void Parse_Parameters( int time, int alarm, int manual);

int Parse_sys_time(char *time);
