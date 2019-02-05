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
 */

#include "KPIWrapper.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <time.h>
#include <unistd.h>
#include <string>
#include "json.hpp"

using namespace KPIWrapper;
using json = nlohmann::json;

ResultSet::ResultSet(KPIWrapper::ResultSetContext newContext, timestamp_nano start, std::string appdatadir) {
    start_timestamp = start;
    setContext(newContext);
    std::string mkdirCMD = "mkdir -p " + appdatadir;
    system(mkdirCMD.c_str());
    mkdirCMD = mkdirCMD + "/cache";
    system(mkdirCMD.c_str());
    appdir = appdatadir;
    const std::string filename = appdir + "running.lock";
    std::ofstream lockfile;
    lockfile.open(filename);
    lockfile << ::getpid() << "\n" << time(0) << std::endl;
}

ResultSet::~ResultSet() {
    std::string filename = appdir + "running.lock";
    remove(filename.c_str());

}

void ResultSet::setContext(KPIWrapper::ResultSetContext newContext) {
    context = newContext;
}

bool ResultSet::stopAndSave(timestamp_nano end_time, std::string dir) {
    end_timestamp = end_time;
    json j;
    char buf[24];
    std::string iso8601_time;

    //Context
    j["client_desc"] = context.client_desc;
    j["client_version"] = context.client_version;
    j["client_os"] = context.client_os;
    j["client_os_version"] = context.client_os_version;
    j["client_id"] = context.client_id;

    //ISO8061 
    std::time_t t = std::chrono::system_clock::to_time_t(start_timestamp);
    std::ostringstream ss;
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(start_timestamp.time_since_epoch()) % 1000000;
    if (0 < strftime(buf, sizeof (buf), "%FT%T.", std::localtime(&t))) {
        ss << buf << std::setfill('0') << std::setw(6) << us.count();
        strftime(buf, sizeof (buf), "%z", std::localtime(&t));
        ss << buf;
        iso8601_time = ss.str();
        iso8601_time.insert(iso8601_time.size() - 2, 1, ':');
    }

    j["start_timestamp"] = iso8601_time;

    t = std::chrono::system_clock::to_time_t(end_timestamp);
    us = std::chrono::duration_cast<std::chrono::microseconds>(end_timestamp.time_since_epoch()) % 1000000;
    ss.str("");
    ss.clear();
    if (0 < strftime(buf, sizeof (buf), "%FT%T.", std::localtime(&t))) {
        ss << buf << std::setfill('0') << std::setw(6) << us.count();
        strftime(buf, sizeof (buf), "%z", std::localtime(&t));
        ss << buf;
        iso8601_time = ss.str();
        iso8601_time.insert(iso8601_time.size() - 2, 1, ':');

    }
    j["end_timestamp"] = iso8601_time;

    //WIFI
    for (auto kv : wifiKPIs.getContext()) {
        j["wifi"][kv.key] = kv.value;
    }
    json wifi_kpis = json::array();
    for (auto kvt : wifiKPIs.getKVTs()) {
        wifi_kpis.push_back({
            {"name", kvt.key},
            {"value", kvt.value},
            {"time_offset", kvt.timeoffset}
        });

    }
    j["wifi"]["wifi_kpis"] = wifi_kpis;


    //L4
    json l4_measurements = json::array();
    for (auto set : l4KPISets) {
        json measure = json::object();
        for (auto kv : set->getContext()) {
            if (kv.key == "time_end_offset" || kv.key == "time_start_offset") {
                try {
                    measure[kv.key] = std::stol(kv.value);
                } catch (...) {
                    measure[kv.key] = kv.value;
                } 
            } else {
                measure[kv.key] = kv.value;
            }


        }
        json kpis = json::array();
        for (auto kvt : set->getKVTs()) {
            kpis.push_back({
                {"name", kvt.key},
                {"value", kvt.value},
                {"time_offset", kvt.timeoffset}
            });
        }
        measure["l4_kpis"] = kpis;
        l4_measurements.push_back(measure);
    }

    //IAD
    j["iad"] = {
        {"iad_desc", iad.iad_desc},
        {"firmware_version", iad.firmware_version},
        {"sync_down", iad.sync_down},
        {"iad_up", iad.sync_up}
    };

    //RTT
    json rttProbings = json::array();
    for (auto set : rttProbingSets) {
        json measure = json::object();
        for (auto kv : set->getContext()) {
            measure[kv.key] = kv.value;
        }
        json kpis = json::array();
        for (auto kvt : set->getKVTs()) {
            kpis.push_back({
                {"name", kvt.key},
                {"value", kvt.value},
                {"time_offset", kvt.timeoffset}
            });
        }
        measure["rtts"] = kpis;
        rttProbings.push_back(measure);
    }
    j["rtt_probings"] = rttProbings;
    j["l4_measurments"] = l4_measurements;
    //std::cout << j.dump(2) << std::endl;


    std::time_t t_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::ostringstream stream;
    if (0 < strftime(buf, sizeof (buf), "%FT%T", std::localtime(&t_now))) {
        stream << buf;

    }
    std::string fileName = dir + "cache/ref-client-measure_" + stream.str() + ".json";
    std::ofstream file(fileName);
    std::cout << fileName << std::endl;
    if (file.is_open()) {
        file << j.dump(2) << std::endl;
        file.close();
    } else {
        std::cerr << "Faild to write data" << std::endl;
        exit(-1);
    }

}

KPIWrapper::KPISet * ResultSet::getWifi() {
    return &wifiKPIs;
}

KPIWrapper::IAD * ResultSet::getIAD() {
    return &iad;
}

KPIWrapper::KPISet * ResultSet::getNewL4() {
    KPISet* set_ptr = new KPISet();
    l4KPISets.push_back(set_ptr);
    return set_ptr;
}

void KPISet::pushKeyValueTimeRetro(std::string key, std::string value, long int timeoffset) {
    KPIWrapper::KeyValueTime kvt;
    kvt.key = key;
    kvt.value = value;
    kvt.timeoffset = timeoffset;
    KPIs.push_back(kvt);
}

void KPISet::setContext(MeasureContext newContext) {
    context = newContext;
}

KPIList KPISet::getKVTs() {
    return KPIs;
}

MeasureContext KPISet::getContext() {
    return context;
}

KPISet::KPISet() {

}
