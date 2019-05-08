// Helper functions

#include "common.h"


using namespace std; 

int connected() {
    int a = connect(sockfd , (struct sockaddr *)&server, sizeof(server));

    if (a < 0) {
	printf("value of a = %d",a);
	printf("\nconnect error");
    } 
    else {
	//printf("value of a = %d",a);
	puts("\nConnected");
    }
    return 0;
}



/*------------------------------------------------------------------------------------------*/
/*---      			 R A N D O M   F U N C T I O N          		 ---*/
/*------------------------------------------------------------------------------------------*/

/*
 * Creating own random number generator using srand()
 * because 1st value of rand() is always same each it is executed
 * as the initial seed is fixed in the defination
*/

int rand_my() {
	srand(time(0));
	//printf("%d\n %d", (rand_my() % 255) + 1);
	return rand();
}



/*-------------------------------------------------------------------------------------------*/
/*---       			B U I L D   U D P   D A T A            			  ---*/
/*-------------------------------------------------------------------------------------------*/

/**
 * Populate the buffer with random data.
 */


void build(uint8_t* buffer, size_t length) {	// size_t is unsigned integer type of at least 16 bit
    for (size_t i = 0; i < length; i++) {
        buffer[i] = (rand_my() % 255) + 1;
    }
}


void build_payload(uint64_t* buffer, size_t length, unsigned long long int msg, int no_of_samples) {
    buffer[0] = msg;
    buffer[1] = 0;
    buffer[2] = no_of_samples;
    for(size_t i = 3; i < length; i++) {
	buffer[i] = 18446744073709551615;
    }
}

bool is_numeric(char *string)
{
    int sizeOfString = strlen(string);
    int iteration = 0;
    bool isNumeric = true;

    while(iteration < sizeOfString)
    {
        if(!isdigit(string[iteration]))
        {
            isNumeric = false;
            break;
        }

        iteration++;

    }

    return isNumeric;
}


void print_help() {
	printf("\nUsage:\t./a.out -s <number of samples>");
	printf("\n\nPossible arguments:\n\t-s, <number of samples>\ttotal number of samples to be measured");
	printf("\n\nIf no argument is provided the default values will be used\n");
	printf("Dafault values are:\n\n\tnumber of samples = infinity\n\n");
	exit(0);
}


void error_check( int error_number) {
	
	if (error_number >= 5000) {
		switch(error_number) {
		    case 5001 :
			printf("\n###\tERROR! The number of samples must be an integer value.\t##\n\n");
			print_help();			
			break;
		}
	}
}



void summery () {

	buffer[0] = 999999;
	sendto(sockfd, &buffer, BUFFER_SIZE, 0, (const struct sockaddr*)&server, sizeof(server));

	close(sockfd);	

	clock_gettime(CLOCK_REALTIME, &time_end);
	
	printf("\nStart time offset: %lld\n", (long long)time_start.tv_sec * 1000000000 + time_start.tv_nsec);

        p = ctime(&time_start.tv_sec); /* ctime() isn't thread-safe. */
  	p[strcspn(p, "\r\n")] = 0;
  	printf("Start Date: %s %ldns\n", p, time_start.tv_nsec);


	printf("\nEnd time offset: %lld\n", (long long)time_end.tv_sec * 1000000000 + time_end.tv_nsec);

        p = ctime(&time_end.tv_sec); /* ctime() isn't thread-safe. */
  	p[strcspn(p, "\r\n")] = 0;
  	printf("End Date: %s %ldns\n", p, time_end.tv_nsec);

	printf("\n########################################################\n");

	if (my_ping(destination_ip) == false) {
		printf("\n\nMeasurement results can not be accepted \nReason: Target disconnected... !\n");
		exit(0);
	}

	printf("\nMeasurement finished...\n\n");
	printf("Average throughput measured %0.1Lf Mbit/s\n", avg/(float)(count_loop - 1));
	printf("Maximum throughput measured %0.1f Mbit/s\n", max_value);
	printf("Minimum throughput measured %0.1f Mbit/s\n\n", min_value);

	
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

}



/*---------------------------------------------------------------------------------------------------*/
/*---  		C R E A T E    J S O N    A N D    U P L O A D   T O    S E R V E R               ---*/
/*---------------------------------------------------------------------------------------------------*/


void push_result () {

    summery();
    //Set measurement context
    KPIWrapper::ResultSetContext context;
    context.client_desc = "2018";
    context.client_version = "1.4";
    context.client_id = "100.01";

    
    std::string cacheDir = "/wiam/project/";

    //Result set
    KPIWrapper::ResultSet result(context, start, cacheDir);

    //Get KPISets references from wrapper
    KPIWrapper::KPISet* l4DataDownEth = result.getNewL4();
    KPIWrapper::KPISet* l4DataUpEth = result.getNewL4();

    time_start_nsec = (long int)time_start.tv_sec * 1000000000 + time_start.tv_nsec;
    time_end_nsec = (long int)time_end.tv_sec * 1000000000 + time_end.tv_nsec;
    time_code_start_nsec = (long int)time_code_start.tv_sec * 1000000000 + time_code_start.tv_nsec;
    //unsigned long int time_intermediate_nsec = (long int)time_intermediate[0].tv_sec * 1000000000 + time_intermediate[0].tv_nsec;

    average = avg / (count_loop - 1);

    l4DataUpEth->pushKeyValueTimeRetro("datarate_max", std::to_string(max_value), (time_end_nsec - time_code_start_nsec)/1e6);
    l4DataUpEth->pushKeyValueTimeRetro("datarate_min", std::to_string(min_value), (time_end_nsec - time_code_start_nsec)/1e6);
    l4DataUpEth->pushKeyValueTimeRetro("datarate_avg", std::to_string(average), (time_end_nsec - time_code_start_nsec)/1e6);

    for (int i = 1; i < count_loop; i++) {
	l4DataUpEth->pushKeyValueTimeRetro("Datarate", std::to_string(throughput_intermediate[i]), (float)(time_intermediate[i] - time_start_nsec) / 1000000); // time in milliseconds
    }
    //std::cout<<time_start<<"    "<<time_end<<std::endl;
    l4DataUpEth->setContext({
                {"sensor_name", "TH Koln Lab"},
                {"sensor_version", "2.019"},
                {"ip_version", "4"},
                {"server", "TH Koeln Lab"},
                {"type", "UDP Discard test"},
                {"time_start_offset", std::to_string((time_start_nsec - time_code_start_nsec) / 1e6)},
                {"time_end_offset", std::to_string((time_end_nsec - time_start_nsec)/1e6)}
            });

	printf("\nTime of code start: %llu",		time_code_start_nsec);
	printf("\ntime of measurement start: %llu",	time_start_nsec);
	printf("\nTime of first iteration: %llu", 	time_intermediate[0]);
	printf("\nTime of second iteration: %llu", 	time_intermediate[1]);
	printf("\nTime of measurement stop: %llu\n\n",	time_end_nsec);
    //Close measurement and let KPIWrapper save the results
    auto end = std::chrono::high_resolution_clock::now();
    result.stopAndSave(end, cacheDir);
    exit(0);
}
