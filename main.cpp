#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include "limit_ob.hpp"

using namespace LOB;

int main() {
    LimitTree<Side::Buy> buytree;
    UID id = 23;
    Quantity qt = 110;

    Order order{id, Side::Sell, qt, 0};
    Order* opt = &order;
    buytree.placeLimit(opt);
    
}