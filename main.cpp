// Create executable file: g++ client.cpp -lpthread -lm -L. -lwiam-kpi-wrapper
// Upload to the server: ./wiam-sync-client thesis/project https://wiam.dn.fh-koeln.de

// Server address: https://wiam.dn.fh-koeln.de/measurementsets/?sort=-server_creation_timestamp&limit=2


// gcc client.c -lpthread -lm
// Link flag -lm tell gcc to link your code against the math lib

/*
 * Static libraries are created by simply archiving the *.o files with the ar program:

 * # Create the object files (only one here)
 * g++ -c client.cpp
 *
 * This will create client.o file
 *
 * # Create the archive (insert the lib prefix)
 * ar rcs libwiam-kpi-wrapper.a client.o
 * This will create a.out file
 *

*/

#include "common.h"
#include "helper.cpp"


using namespace std;



/*----------------------------------------------------------------------------*/
/*--- 			 S O C K E T   T H R E A D         	           ---*/
/*----------------------------------------------------------------------------*/

static void *thread_socket(void *_) {


    //printf("Build Data...\n");
    build((uint8_t*)buffer, sizeof(buffer));

    //printf("Configure socket...\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);	// SOCK_DGRAM, SOCK_STREAM
    if (sockfd < 0)
    {
        fprintf(stderr, "Error opening socket");
        exit(1);
    }

    /*
    if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &snd_buffer, sizeof(snd_buffer)) < 0) {
	fprintf(stderr, "Error setting Send buffer size.");
        exit(1);
    }*/

	/*
    // Get buffer size
    optlen = sizeof(sendbuff);
    res = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);
    printf()*/


/*----------------------------------------------------*/
/*---       Initialize address protocol            ---*/
/*----------------------------------------------------*/

    bzero((char*)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVERADDRESS);
    server.sin_port = htons(PORT);

    /*char clntIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (struct inaddr *)&client.sin_addr,clntIP,sizeof(clntIP));
    printf("client IP is %s\n",clntIP);*/

    socklen_t len = sizeof(sockaddr_in);
    getpeername(sockfd, (struct sockaddr*)&client, &len);
    char clntIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (struct inaddr *)&client.sin_addr,clntIP,sizeof(clntIP));
    printf("client IP is %s\n",clntIP);

    connected();
	
    /*int check = connected();
    if (check == -1) {
	printf("\nfunction value : %d",check);
	printf("\nconnect error");
    }
    else {
	printf("\nfunction value : %d",check);
	puts("\nConnected ok");
	}*/

/*---------------------------------------------------*/
/*--- 		S E N D I N G   D A T A 	   --*/
/*---------------------------------------------------*/

    //printf("\nSend UDP data...\n\n");

    //for (int j = 0; j < BUFFER_SIZE; j += UDP_FRAME) {
    //for (j=0; j < sample_numbers; j++) { 
    while(1) {	

        if (sendto(sockfd, &buffer, UDP_FRAME, 0,
                    (const struct sockaddr*)&server, sizeof(server)) < 0)
        {
            //fprintf(stderr, "Error in sendto()\n");
            //return EXIT_FAILURE;
        }
	count_frame += 1;    
    }
    close(sockfd);

}


/*----------------------------------------------------------------------------*/
/*--- 		     T H R O U G H P U T   T H R E A D        	           ---*/
/*----------------------------------------------------------------------------*/


static void * thread_throughput(void * _) {
    
    //pthread_mutex_lock(&lock);

    struct timespec start, end, new_start;
    long double t_start, t_end;

    end.tv_nsec = 0;
    start.tv_nsec = 0;
    new_start.tv_nsec = 0;
    //sleep(1);
    count_frame = 0;

    clock_gettime(CLOCK_REALTIME, &time_start_offset);

   
    while(1) {
	

	//sleep(1);
        usleep(count_time * 1000);	// converts micro sleep to milisleep

	start = new_start;
	clock_gettime(CLOCK_REALTIME, &new_start);


	delta_frame = count_frame - old_frame;	
	old_frame = count_frame;

	//count_frame=0;
	delta_ns = (new_start.tv_sec * 1000000000 + new_start.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec);

	/*
	if (new_start.tv_nsec > start.tv_nsec) {	
		delta_ns = (new_start.tv_nsec - start.tv_nsec);
	}
	else
		delta_ns = (start.tv_nsec - new_start.tv_nsec);
	*/
		

	//throughput_Byte = (float)(count_frame * 1442) / (delta_ns/ 1e9f));	
	//throughput_bit = (8 * (float)(count_frame * 1442) / (delta_ns/ 1e9f));
	throughput_Mbit = floor(((8 * (float)(delta_frame * UDP_FRAME) / (delta_ns/ 1e9f)) / 1000000)*100) / 100;



	if (count_loop != 0.00) {

	printf("\nInterval between loops  : %f[s]",delta_ns / 1e9f);
	printf("\nTotal datagrams sent: %llu\n", delta_frame * UDP_FRAME);

	throughput = throughput_Mbit;

	if(throughput >= max_value) 
		max_value = throughput;

	if(throughput <= min_value && throughput > 0.00) 
		min_value = throughput;

	avg = avg + throughput;

			
	printf("\u2588\u2588");
	for(int i=0; i<=(int)(throughput); i++) {		
		printf("\u2591");
	}

	printf(" Throughput %0.1f Mbit/s\n", throughput);
	printf("\u2588\u2588\n");
	

	total_time += (float)count_time / 1000.000;	// in seconds
	}
	
	if (throughput > 95.623) {
		trace+=1;
	}

	if (throughput <= 95.623 && throughput > 94.9) {
		trace_95 += 1;
	}

	if (throughput <= 94.9)
		trace_94 += 1;

	time_intermediate[count_loop] = (long int)new_start.tv_sec * 1000000000 + new_start.tv_nsec;
	throughput_intermediate[count_loop] = throughput;
	count_loop ++;
	if (count_loop == sample_numbers) {
		push_result();
		//putchar(getchar());
		//puts("\n");
	}


    }
    //pthread_mutex_unlock(&lock);
}

/*----------------------------------------------------------------------------*/
/*--- 			 S U M M A R Y   T H R E A D         	           ---*/
/*----------------------------------------------------------------------------*/

static void * thread_summery(void * _) {
	while(getchar() != '\n' && getchar() != getchar());
	
	next:
	close(sockfd);	

	clock_gettime(CLOCK_REALTIME, &time_end_offset);
	printf("\nstart time: %lld\n", (long long)time_start_offset.tv_sec * 1000000000 + time_start_offset.tv_nsec);
	printf("\nstart time: %lld\n", (long long)time_end_offset.tv_sec * 1000000000 + time_end_offset.tv_nsec);
	printf("\n########################################################\n");
	printf("\nMeasurement finished...\n\n");
	printf("Maximum throughput measured %0.1f Mbit/s\n", max_value);
	printf("Minimum throughput measured %0.1f Mbit/s\n", min_value);
	printf("Average throughput measured %0.1Lf Mbit/s\n\n", avg/(float)(count_loop - 1));
	printf("Throughput error value : %f %% \n", (trace * 100) / (float)(count_loop) );
	printf("Number of occurances of error value: %0.f\n\n", trace);

	printf("Throughput value within 95 and 95.623 : %f %% \n", (trace_95 * 100) / (float)(count_loop) );
	printf("Number of occurances within 95 and 95.623 : %0.f\n\n", trace_95);

	printf("Throughput value less than 95 : %f %% \n", (trace_94 * 100) / (float)(count_loop) );
	printf("Number of occurances less than 95 : %0.f\n\n", trace_94);
	
	if (total_time >= 60 && total_time < 3600) {
		duration = total_time / 60;
		printf("Total duration %0.0f minutes %d seconds %0.0f milliseconds \n\n", duration, ((int)total_time % 60), (total_time - (float)((int)total_time)) * 1000);
	}

	else if (total_time >= 3600) {
		duration = total_time / 3600;
		printf("Total duration %0.0f hours %d minutes %d seconds %0.0f milliseconds \n\n", duration, ((int)total_time % 3600) / 60 , ((int)total_time % 60), (total_time - (float)((int)total_time)) * 1000);
	}
	
	else
		printf("Total duration %0.0f seconds %0.0f milliseconds\n\n", total_time, (total_time - (float)((int)total_time)) * 1000);
		
	printf("########################################################\n\n");
	push_result();

	exit(0);	
	}


/*----------------------------------------------------------------------------*/
/*--- 			 M A I N   F U N C T I O N         	           ---*/
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	/**
	 * You have no idea what order initial threads will be done with first
	 */
	/*if (pthread_mutex_init(&lock, NULL) != 0) {
        	printf("\n mutex init failed\n");
        	return 1;
    	}*/

	if (argc != 2) {
      		printf ("Required arguments: ./a.out <Number of samples>\n");
      	exit (0);
    	}

	sample_numbers = atoi(argv[1]);
	pthread_create(&t[1], NULL, thread_socket, NULL);
	pthread_create(&t[0], NULL, thread_throughput, NULL);
	pthread_create(&t[2], NULL, thread_summery, NULL);

	//pthread_mutex_destroy(&lock);

	//must have this as main will block until all the supported threads are done
	pthread_exit(NULL);


	return 1;
}
