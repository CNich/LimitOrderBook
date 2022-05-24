#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include "limit_ob.hpp"
#include <chrono>

using namespace LOB;

int main() {
    /*
    LimitTree<Side::Buy> buytree;
    UID id = 23;
    Quantity qt = 110;

    Order order{id, Side::Sell, qt, 0};
    Order* opt = &order;
    buytree.placeLimit(opt);
    */

    // aim: 4k executions per sec
    // execution = order?
    std::default_random_engine gen;
    auto price = std::normal_distribution<double>(500, 20);
    auto quantity = std::normal_distribution<double>(1000,200);
    LimitOrderBook limitob;
    int num_orders = 4000000;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < num_orders; i++) {
        auto p = static_cast<Price>(price(gen));
        auto q = static_cast<Quantity>(quantity(gen));
        limitob.limit(Side::Buy, 0, p, q);
        limitob.cancel(0);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    
    std::cout<<"Time taken to submit "<<num_orders<<" orders: " \
    << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< "us" << std::endl;
    return 0;
    
}