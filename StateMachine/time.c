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


// This function convert the hour and mins to mins in 24 hrs base
int time_transfer(int hour, int min){
	return hour*60 + min;

}

// This function parse input from a config file to the int hour and mins

int Parse_InputTime(){
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
	int total_time;
	if(fp==NULL){
		printf("\nError: Can't find file\n");
		exit(0);
	}
	
	Token = strtok(buff, ":");
	// Seperate by space: to form one HH:MM;1/0;1/0
	hr = atoi(Token);
		
	Token = strtok(NULL, ";");
	min = atoi(Token);
		
	
	
	Token = strtok(NULL, ";");
	//alarm = atoi(Token);
	//printf("\nalarm:%d\n",alarm);
	Token = strtok(NULL, ";");
	//manual = atoi(Token);
	//printf("\nmanual:%d\n",manual);
	 
	//printf("\ntime:%d\n",time);
	
	fclose(fp);
	return time_transfer(hr,min);
}

int Parse_Alarm(){
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
	int total_time;
	if(fp==NULL){
		printf("\nError: Can't find file\n");
		exit(0);
	}
	
	Token = strtok(buff, ":");
	// Seperate by space: to form one HH:MM;1/0;1/0
	hr = atoi(Token);
		
	Token = strtok(NULL, ";");
	min = atoi(Token);
		
	
	
	Token = strtok(NULL, ";");
	int alarm = atoi(Token);
	//printf("\nalarm:%d\n",alarm);
	Token = strtok(NULL, ";");
	//manual = atoi(Token);
	//printf("\nmanual:%d\n",manual);
	 
	//printf("\ntime:%d\n",time);
	
	fclose(fp);
	return alarm;
}

int Parse_Manual(){
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
	int total_time;
	if(fp==NULL){
		printf("\nError: Can't find file\n");
		exit(0);
	}
	
	Token = strtok(buff, ":");
	// Seperate by space: to form one HH:MM;1/0;1/0
	hr = atoi(Token);
		
	Token = strtok(NULL, ";");
	min = atoi(Token);
		
	
	
	Token = strtok(NULL, ";");
	//alarm = atoi(Token);
	//printf("\nalarm:%d\n",alarm);
	Token = strtok(NULL, ";");
	int manual = atoi(Token);
	//printf("\nmanual:%d\n",manual);
	 
	//printf("\ntime:%d\n",time);
	
	fclose(fp);
	return manual;
}
//Parse system time
int Parse_sys_time(char *time){
	int totMin = 0;
	int min = 0;
	int hr = 0;
	int sec = 0;
	
	char* Token;
	char sys_time[1024];
	strcpy(sys_time,__TIME__);
	
	Token = strtok(sys_time, ":");
	hr = atoi(Token);

	Token = strtok(NULL, ":");
	min = atoi(Token);
	Token = strtok(NULL, ":");
	sec = atoi(Token);
	Token = strtok(NULL, ":");
	
	totMin = time_transfer(hr, min);
	return totMin;
	
	
	
}




int Parse_State(char *time){
	
	
	
	char* Token;
	char sys_time[1024];
	strcpy(sys_time,time);
	
	Token = strtok(sys_time, " ");
	
	if(strcmp(Token,"DEEP")==0){
		
		return DEEP;
		printf("STATE:%s\n",Token);
	} else if(strcmp(Token,"LIGHT")==0){
		
		return LIGHT;
	} else if(strcmp(Token,"DONE")==0){
		
		return DONE;
	}

	Token = strtok(NULL, " ");
	
	
	Token = strtok(NULL, " ");
	
	

}

int Parse_Brightness(char *time){
	
	
	
	char* Token;
	char sys_time[1024];
	strcpy(sys_time,time);
	
	Token = strtok(sys_time, " ");
	

	Token = strtok(NULL, " ");
	printf("Token:%s\n",Token);
	return (int) atof(Token);
	
	Token = strtok(NULL, " ");
	
	

}