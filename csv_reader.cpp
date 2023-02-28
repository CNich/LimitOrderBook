#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <array>
#include <time.h>
#include "limit_ob.hpp"


int main(int argc, char *argv[]) {
    LOB::LimitOrderBook limitob;
    LOB::LimitOrderBook limitob_control;
    std::cout<<"==============================\n";
    std::cout<<argv[1]<<"\n";
    std::cout<<"==============================\n";
    std::string base_filename = std::string(argv[1]);
    auto snap_csv = base_filename + "_snap.csv";
    auto depth_csv = base_filename + "_update.csv";


    std::cout<<"loaded snap\n";
    limitob.load_csv(1, snap_csv.c_str(), 0, -1, "");
    std::cout<<"loaded update\n";
    limitob.load_csv(1, depth_csv.c_str(), 1, 0, base_filename);

    return 0;
}

int test(int argc, char *argv[]) {
    // "../binance_csv_data/BTCUSDT_T_DEPTH_2021-10-30/BTCUSDT_T_DEPTH_2021-10-30_depth"
    std::string base_filename = std::string(argv[1]);
    auto snap_csv = base_filename + "_snap.csv";
    auto depth_csv = base_filename + "_update.csv";

    LOB::LimitOrderBook limitob;
    LOB::LimitOrderBook limitob_control;
    // uint64_t init_snap_ts = 1635552289073;
    uint64_t max_ts = 1635557445953;

    std::cout<<"loaded snap WITHOUT init_snap_ts\n";
    limitob.load_csv(1, snap_csv.c_str(), 0, -1, "");
    std::cout<<"loaded update\n";
    limitob.load_csv(1, depth_csv.c_str(), 0, max_ts, base_filename);
    std::cout<<"loading control\n";
    limitob_control.set_min_ts(max_ts);
    limitob_control.load_csv(1, snap_csv.c_str(), 0, max_ts, base_filename);

    LOB::Price p[11] = {61998740, 61997210, 61996090, 61995670, 61995470, 61995010, 61995000, 61994200, 61993910, 61993900, 61992080};
    int num_errors = 0;
    for(int i = 0; i < 11; i++){
        LOB::Quantity q = limitob.quantity_at(LOB::Side::Buy, p[i]);
        LOB::Quantity qc = limitob_control.quantity_at(LOB::Side::Buy, p[i]);
        if(q != qc){
            num_errors++;
        }
        std::cout<<p[i] << " " << q << " " << qc << " " << q-qc << "\n";
    }
    std::cout<<"num_errors " << num_errors << "\n";

    LOB::Price p2[11] ={61998750, 61999880, 61999940, 62000000, 62000120, 62000160, 62000500, 62000650, 62001200, 62001350, 62001530};
    num_errors = 0;
    for(int i = 0; i < 11; i++){
        LOB::Quantity q = limitob.quantity_at(LOB::Side::Sell, p2[i]);
        LOB::Quantity qc = limitob_control.quantity_at(LOB::Side::Sell, p2[i]);
        if(q != qc){
            num_errors++;
        }
        std::cout<<p[i] << " " << q << " " << qc << " " << q-qc << "\n";
    }
    std::cout<<"num_errors " << num_errors << "\n";

    return 0;
}

