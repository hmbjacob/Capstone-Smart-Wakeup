// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080
#include "time.h"
int main(int argc, char const *argv[])
{	
	char prev[1024] = {0};
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	int Ready_print=0;
	send(sock , hello , strlen(hello) , 0 );
	while(1){
		valread = read( sock , buffer, 1024);
		printf(" %s \n", buffer);
		/*
		for (int j=0;j<sizeof(buffer[0]);j++){
			if(prev[j]!= buffer[j]){
				Ready_print = 1;
			}
		}
		if(Ready_print==1){
			printf("%s\n", buffer);
			Ready_print  = 0;
		}
		
		
		for (int i=0;i<sizeof(buffer[0]);i++){
			prev[i] = buffer[i];
		}*/
			
	}
	
	return 0;
}
