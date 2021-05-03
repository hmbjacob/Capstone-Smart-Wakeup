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


// This function convert the hour and mins to mins in 24 hrs base
int time_transfer(int hour, int min){
	return hour*60 + min;

}

// This function parse input from a config file to the int hour and mins

void Parse_Parameters(int time, int alarm, int manual){
	FILE* fp;
	char buff[1024];
	fp = fopen("config.txt","r");
	fgets(buff,1024,fp);
	char* Token;
	char* Token2;
	char* Token3;
	int index = 0;
	int subindex = 0;
	int hr;
	int min;

	if(fp==NULL){
		printf("\nError: Can't find file\n");
		exit(0);
	}
	
	Token = strtok(buff, ":");
	// Seperate by space: to form one HH:MM;1/0;1/0


	
		

	hr = atoi(Token);
		
	Token = strtok(NULL, ";");
	min = atoi(Token);
		
	time = time_transfer(hr,min);
	printf("\ntime:%d\n",time);
	Token = strtok(NULL, ";");
	alarm = atoi(Token);
	printf("\nalarm:%d\n",alarm);
	Token = strtok(NULL, ";");
	manual = atoi(Token);
	printf("\nmanual:%d\n",manual);




	fclose(fp);
	
}