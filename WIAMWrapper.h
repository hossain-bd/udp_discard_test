/*
#################################################################################
#  _______  _    _   _  __ _   _  _        					#
# |__   __|| |  | | | |/ /(_) (_)| |       Technische Hochschule Koeln		#
#    | |   | |__| | | ' /   ___  | | _ __  Cologne University of Applied Science#
#    | |   |  __  | |  <   / _ \ | || '_ \ 					#
#    | |   | |  | | | . \ | (_) || || | | |Institute of Communication Systems	#
#    |_|   |_|  |_| |_|\_\ \___/ |_||_| |_|Lab for Datanetworks  		#
#       Authors:                                                                #
#		Constantin Eiling constantin.eiling@th-koeln.de			#
#                                                                               #
#       Website: http://wiam.tools                                              #
#                                                         	                #
#       Copyright (C) 2018                     			                #
#                                                                               #
#       Technische Hochschule Koeln - Cologne University of Applied Sciences    #
#       www.th-koeln.de / www.dn.th-koeln.de / 				        #
#                                                                               #
#################################################################################

/* 
 * File:   WIAMWrapper.h
 * Author: ceiling
 *
 * Created on 21. August 2018, 11:07
 */

#ifndef KPIWRAPPER_H
#define KPIWRAPPER_H



#include <string>
#include <vector>
#include <chrono>
#include <ctime>
namespace KPIWrapper {

    typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> timestamp_nano;

    struct ResultSetContext {
        std::string client_desc = "-";
        std::string client_version = "-";
        std::string client_os = "-";
        std::string client_os_version = "-";
        std::string client_id = "-";
    };

    struct KPISetContext {
    protected:
        KPISetContext();
    };

    struct IAD {
        std::string iad_desc = "-";
        std::string firmware_version = "-";
        int sync_down = -1;
        int sync_up = -1;
    };

    struct KeyValueTime {
        std::string key;
        std::string value;
        long int timeoffset;
    };

    struct KeyValue {
        std::string key;
        std::string value;
    };

    typedef std::vector<KPIWrapper::KeyValueTime> KPIList;
    typedef std::vector<KPIWrapper::KeyValue> MeasureContext;

    class KPISet {
    public:
        KPISet();
        //void pushKeyValueTime(std::string key, std::string value);
        void pushKeyValueTimeRetro(std::string key, std::string value, long int timeoffset);
        void setContext(MeasureContext newContext);
        KPIList getKVTs();
        MeasureContext getContext();
    private:
        KPIWrapper::KPIList KPIs;
        KPIWrapper::MeasureContext context;


    };

    class ResultSet {
    public:
        bool stopAndSave(timestamp_nano stop_time, std::string dir);
        KPIWrapper::KPISet* getWifi();
        KPIWrapper::IAD* getIAD();
        KPIWrapper::KPISet* getNewL4();
        KPIWrapper::KPISet* getNewRTTProbing();
        ResultSet(KPIWrapper::ResultSetContext newContext, timestamp_nano start_time, std::string appdatadir);
        ~ResultSet();
        void setContext(KPIWrapper::ResultSetContext newContext);
    private:
        KPIWrapper::ResultSetContext context;
        KPIWrapper::KPISet wifiKPIs;
        std::vector<KPIWrapper::KPISet*> l4KPISets;
        std::vector<KPIWrapper::KPISet*> rttProbingSets;
        KPIWrapper::IAD iad;
        timestamp_nano start_timestamp;
        timestamp_nano end_timestamp;
        bool startable = false;
        std::string appdir;
    };
}


#endif /* WIAMWRAPPER_H */

