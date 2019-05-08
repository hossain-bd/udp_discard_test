// Create executable file: g++ client.cpp -lpthread -lm -L. -lwiam-kpi-wrapper -o wiam_udp_discard
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
 *
 *usage: ./wiam_udp_discard -ip 192.168.178.27 -s 10
 *
 *

*/

#include "common.h"
#include "helper.cpp"



using namespace std;




/*----------------------------------------------------------------------------*/
/*--- 			 S O C K E T   T H R E A D         	           ---*/
/*----------------------------------------------------------------------------*/

static void *thread_socket(void * _) {



    //build((uint8_t*)buffer, sizeof(buffer));

    //printf("Configure socket...\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);	// SOCK_DGRAM, SOCK_STREAM
    if (sockfd < 0)
    {
        fprintf(stderr, "Error opening socket");
        exit(1);
    }


/*----------------------------------------------------------------------------*/
/*---       		Initialize address protocol     	           ---*/
/*----------------------------------------------------------------------------*/

    bzero((char*)&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(destination_ip);
    server.sin_port = htons(PORT);


    socklen_t len = sizeof(sockaddr_in);
    getpeername(sockfd, (struct sockaddr*)&client, &len);
    char clntIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (struct inaddr *)&client.sin_addr,clntIP,sizeof(clntIP));


/*----------------------------------------------------------------------------*/
/*--- 			S E N D I N G   D A T A 			    --*/
/*-------------------------------------------------------
---------------------*/

    unsigned long long int msg = 0;

    clock_gettime(CLOCK_REALTIME, &send_start);
    while(1) {	

 	//buffer = msg;
	build_payload((uint64_t*)buffer, 8, msg, (int) sample_numbers);
	

        if (sendto(sockfd, &buffer, sizeof(buffer), 0,
                    (const struct sockaddr*)&server, sizeof(server)) < 0)
        {
            //fprintf(stderr, "Error in sendto()\n");
            //return EXIT_FAILURE;
        }
	count_frame += 1; 
	msg += 1;   
    }
    close(sockfd);

}

/*----------------------------------------------------------------------------*/
/*--- 		     T H R O U G H P U T   T H R E A D        	           ---*/
/*----------------------------------------------------------------------------*/


static void * thread_throughput(void * _) {
    
    //pthread_mutex_lock(&lock);

    struct timespec prev_time, new_start;

    prev_time.tv_nsec = 0;
    new_start.tv_nsec = 0;
    //sleep(1);
    count_frame = 0;

    clock_gettime(CLOCK_REALTIME, &time_start);
    auto start = std::chrono::high_resolution_clock::now();

   
    while(1) {

	//sleep(1);
        usleep(count_time * 1000);	// converts microsleep to milisleep

	prev_time = new_start;
	clock_gettime(CLOCK_REALTIME, &new_start);


	delta_frame = count_frame - old_frame;
	clock_gettime(CLOCK_REALTIME, &intermediate_time);
		
	printf("\nTime passed from starting: %f\n", ((intermediate_time.tv_sec * 1e9 + intermediate_time.tv_nsec) - (send_start.tv_sec * 1e9 + send_start.tv_nsec)) / 1e9);

	old_frame = count_frame;

	//count_frame=0;
	delta_ns = (new_start.tv_sec * 1e9 + new_start.tv_nsec) - (prev_time.tv_sec * 1e9 + prev_time.tv_nsec);
		

	//throughput_Byte = (float)(count_frame * 1442) / (delta_ns/ 1e9f));	
	//throughput_bit = (8 * (float)(count_frame * 1442) / (delta_ns/ 1e9f));
	throughput_Mbit = floor(((8 * (float)(delta_frame * UDP_FRAME) / (delta_ns/ 1e9f)) / 1e6)*100) / 100;

	//throughput_Mbit = floor((8 * (float)(delta_frame * UDP_FRAME) / (delta_ns/ 1e9f)) / 1e6);

	if (count_loop != 0.00) {

	printf("\nInterval between loops  : %f[s]",delta_ns / 1e9f);
	printf("\nTotal frames sent: %llu\n", delta_frame);

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

	printf(" Throughput %llu: %0.2f Mbit/s\n", count_loop, throughput);
	printf("\u2588\u2588\n");
	

	total_time += (float)count_time / 1000.000;	// in seconds
	}

	time_intermediate[count_loop] = (long int)new_start.tv_sec * 1000000000 + new_start.tv_nsec;
	throughput_intermediate[count_loop] = throughput;
	count_loop ++;
	if (count_loop == sample_numbers+1) {
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

	summery();
	exit(0);	
	}


/*----------------------------------------------------------------------------*/
/*--- 			 M A I N   F U N C T I O N         	           ---*/
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	clock_gettime(CLOCK_REALTIME, &time_code_start);

	/**
	 * You have no idea what order initial threads will be done with first
	 */

	
	if (argc < 2) {

		print_help;
    	}


	for(int k=0;k<argc;k++) {

		if ( ! strcmp(argv[k], "-s")) { 
			if (is_numeric(argv[k+1]) == false) {
				error_number = 5001;
				error_check(error_number);
			}
			else {
				sample_numbers = atoi(argv[k+1]);

			}
		}

		if ( ! strcmp(argv[k], "-ip")) { 
			strcpy(destination_ip, argv[k+1]);
		}
		
		if ( ! strcmp(argv[k], "--help")) { 
			print_help();
		}
	}


	printf("Starting ping test...");
	check_ping = my_ping(destination_ip);
	while(check_ping == false) {
		printf("\nTarget mechine can not be reached...!\n");
		check_ping = my_ping(destination_ip);
	}
	if (check_ping == true) {
		printf("\nTarget mechine can be reached\n");
	}	

	// Label threads
	threads:
	pthread_create(&t[1], NULL, thread_socket, NULL);
	pthread_create(&t[0], NULL, thread_throughput, NULL);
	pthread_create(&t[2], NULL, thread_summery, NULL);

	//pthread_mutex_destroy(&lock);

	//must have this as main will block until all the supported threads are done
	pthread_exit(NULL);


	return 1;
}
