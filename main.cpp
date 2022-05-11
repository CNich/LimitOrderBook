#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include "limit_ob.hpp"

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
    for (int i = 0; i < 4000; i++) {
        auto p = static_cast<Price>(price(gen));
        auto q = static_cast<Quantity>(quantity(gen));
        limitob.limit(Side::Buy, 0, 100, 500);
        // what to print?
    }
    

    
}