#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include "limit_ob.hpp"
#include <chrono>

using namespace LOB;

int main() {

    // aim: 4k executions per sec
    // execution = order?
    std::default_random_engine gen;
    auto buy_price = std::normal_distribution<double>(5000, 20);
    auto buy_quantity = std::normal_distribution<double>(1000,200);
    auto sell_price = std::normal_distribution<double>(5500, 20);
    auto sell_quantity = std::normal_distribution<double>(1000,200);
    LimitOrderBook limitob;
    int num_orders = 4000000;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < num_orders; i++) {
        auto bp = static_cast<Price>(buy_price(gen));
        auto bq = static_cast<Quantity>(buy_quantity(gen));
        auto sp = static_cast<Price>(sell_price(gen));
        auto sq = static_cast<Quantity>(sell_quantity(gen));
        limitob.limit(Side::Buy, i, 1, bp);
        limitob.limit(Side::Sell, i+num_orders, 1, sp);
        //limitob.limit_sell_L2(i, q, p);
        //limitob.limit_buy_L2(i, bq, bp);
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout<<"Time taken to submit "<<num_orders<<" orders: " \
    << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()<< "us" << std::endl;

    double facs[5] = {0.0, 0.01, 0.02, 0.05, 0.1};
    limitob.market_impact(Side::Buy, facs, 5);
    limitob.market_impact(Side::Sell, facs, 5);

    limitob.print_sorted(Side::Buy);

    return 0;

}
