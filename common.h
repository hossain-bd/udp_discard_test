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

#define SERVERADDRESS "139.6.19.198"
#define PORT (9)
  
#define NUMTHRDS	3
#define count_time	1000 // milliseconds


#define FRAMES (1)	// 1 for while loop
#define UDP_FRAME (1442)

#define BUFFER_SIZE (FRAMES * UDP_FRAME)


// Data (1442) + UDP (8) + IP (20) + Ethernet (14) = 1484 bytes

// Data (1442) + UDP (8) + IP (20) + [ Ethernet (14) + (4) ] = 1484 bytes


struct timespec time_start_offset, time_end_offset;
struct sockaddr_in server, client, clntIP;
int sockfd, frames_total, i = 0, j = 0, sample_numbers = 0;
pthread_t t [ NUMTHRDS ];
unsigned long long int total_dgram, count_frame = 0, old_frame = 0, delta_frame = 0, count_loop = 0, time_intermediate[5000];
char buffer[BUFFER_SIZE], snd_buffer[100000000], ch;
float throughput, throughput_Byte, throughput_bit, throughput_Mbit, max_value = 0.00, min_value = 1000.00, total_time, duration, trace=0.00, trace_95=0.00, trace_94=0.00, throughput_intermediate[5000];
long double avg = 0.00, average = 0.0;
uint64_t delta_ns;


#endif /* COMMON_H */
