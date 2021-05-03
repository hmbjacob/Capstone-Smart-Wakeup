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
	int time, alarm, manual;

	printf("\n\ntest calculate function\n");
	printf("result:%d\n",time_transfer(10,10));
	printf("end test\n\n");
	Parse_Parameters(time, alarm, manual);
}