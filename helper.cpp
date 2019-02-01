// Helper functions

#include "common.h"


int connected() {
    int a = connect(sockfd , (struct sockaddr *)&server, sizeof(server));

    if (a < 0) {
	printf("value of a = %d",a);
	printf("\nconnect error");
    } 
    else {
	printf("value of a = %d",a);
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



/*---------------------------------------------------------------------------------------------------*/
/*---  		C R E A T E    J S O N    A N D    U P L O A D   T O    S E R V E R               ---*/
/*---------------------------------------------------------------------------------------------------*/

void push_result () {
    //Set measurement context
    KPIWrapper::ResultSetContext context;
    context.client_desc = "2018";
    context.client_version = "1.4";
    context.client_id = "100";
    
    auto start = std::chrono::high_resolution_clock::now();
    std::string cacheDir = "/home/hossain/thesis/project/";

    //Result set
    KPIWrapper::ResultSet result(context, start, cacheDir);

    //Get KPISets references from wrapper
    KPIWrapper::KPISet* l4DataDownEth = result.getNewL4();
    KPIWrapper::KPISet* l4DataUpEth = result.getNewL4();

    unsigned long int time_start = (long int)time_start_offset.tv_sec * 1000000000 + time_start_offset.tv_nsec;
    unsigned long int time_end = (long int)time_end_offset.tv_sec * 1000000000 + time_end_offset.tv_nsec;

    average = avg / (count_loop - 1);

    l4DataUpEth->pushKeyValueTimeRetro("datarate_max", std::to_string(max_value), time_end);
    l4DataUpEth->pushKeyValueTimeRetro("datarate_min", std::to_string(min_value), time_end);
    l4DataUpEth->pushKeyValueTimeRetro("datarate_avg", std::to_string(average), time_end);

    for (int i = 1; i < count_loop; i++) {
	l4DataDownEth->pushKeyValueTimeRetro("Datarate", std::to_string(throughput_intermediate[i]), (float)(time_intermediate[i] - time_start) / 1000000); // time in milliseconds
    }
	
    l4DataDownEth->setContext({
                {"sensor_name", "TH Koln Lab"},
                {"sensor_version", "2.019"},
                {"ip_version", "4"},
                {"server", "TH Koeln Lab"},
                {"type", "UDP Discard test"},
                {"time_start_offset", std::to_string(time_start)},
                {"time_end_offset", std::to_string(time_end)}
            });


    //Close measurement and let KPIWrapper save the results
    auto end = std::chrono::high_resolution_clock::now();
    result.stopAndSave(end, cacheDir);
    exit(0);

}
