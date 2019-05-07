#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <math.h>

#include <math.h>
#include "KPIWrapper.h"
#include "KPIWrapper.cpp"
#include <chrono>

 
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 

#include "my_ping.cpp"
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>

using namespace std; 

#define SERVERADDRESS "139.6.19.198"
#define PORT (9)
  
#define NUMTHRDS	3



#define FRAMES (1)	// 1 for while loop
#define UDP_FRAME (1440)

#define BUFFER_SIZE (FRAMES * UDP_FRAME)


// Data (1442) + UDP (8) + IP (20) + Ethernet (14) = 1484 bytes

// Data (1442) + UDP (8) + IP (20) + [ Ethernet (14) + (4) ] = 1484 bytes

bool 			check_ping;


char 			arg_ip[] = "-ip", 
			arg_samples[] = "-s", 
			arg_help[] = "--help", 
			*p, *strToTest, 
			destination_ip[1000];


struct timespec 	time_start, 
			time_end,
			time_code_start;

struct sockaddr_in 	server, 
		   	client, clntIP;

int 			sockfd,
    			frames_total, 
    			i = 0, j = 0, 
    			count_time = 1000, 
    			error_number = 0; // count_time defined in milliseconds

double 			sample_numbers = -INFINITY;

pthread_t 		t [ NUMTHRDS ];

unsigned long long int 	total_dgram, buffer[180],
		       	count_frame = 0, 
		       	old_frame = 0, 
		       	delta_frame = 0, 
		       	count_loop = 0, 
		       	time_intermediate[5000], 
			time_start_nsec,
			time_end_nsec,
			time_code_start_nsec;

char 			//buffer[BUFFER_SIZE], 
     			snd_buffer[100000000], 
     			ch;

float 			throughput, 
      			throughput_Byte, 
      			throughput_bit, 
      			throughput_Mbit, 
      			max_value = 0.00, 
      			min_value = 1000.00, 
      			total_time, 
      			duration, 
      			trace=0.00, 
      			trace_95=0.00, 
      			trace_94=0.00, 
      			throughput_intermediate[5000];

long double 		avg = 0.00, 
	    		average = 0.0;

uint64_t 		delta_ns;

auto start = std::chrono::system_clock::now();


#endif /* COMMON_H */
