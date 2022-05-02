#include "binary_search_tree.hpp"
#include "LOB_buildingBlocks.hpp"
#include <unordered_map>
//#include <boost/container/falt_map>
//using ska::flat_hash_map; // throws weird ass error 


namespace LOB {

typedef std::unordered_map<Price, Limit*> PriceLimitMap;
// might have to change map, check:
// https://medium.com/applied/gist-better-than-unordered-map-1ad07b0a81b7
// for hash map performances

// ---------------------------------------------- //
// I. functions to be used in limit tree struct:  //
// ---------------------------------------------- //

// I.a. set best price for buy/sell:
template<Side side>
void set_best(Limit** best, Limit* limit);

// set best buy price
// highest buy order is best, 거래 반대편에 있는
// 사람에게 best price
template<>
void set_best<Side::Buy>(Limit** highest_buy, Limit* limit) {
    // if highest buy is null set highest buy points to input limit
    if (*highest_buy == nullptr) *highest_buy = limit;
    // highest_buy points to input limit if limit price is higher
    else if (limit->key > (*highest_buy)->key) *highest_buy = limit;
}
// set best sell price, overload func
template<>
void set_best<Side::Sell>(Limit** lowest_sell, Limit* limit) {
    // if lowest sell is null, set limit 
    if (*lowest_sell == nullptr) *lowest_sell = limit;
    else if (limit->key < (*lowest_sell)->key) *lowest_sell = limit;
}

// I.b. can buy/sell orders match?

// whether or not buy and sell orders can match
template<Side side>
bool can_match(Price limit, Price market);

template<>
bool can_match<Side::Buy>(Price limit, Price market) {
    // buy order wants limit price or lower
    // if market price is 0 then order always can match
    return ( (market == 0) || (market <= limit) );
}

template<>
bool can_match<Side::Sell>(Price limit, Price market) {
    // sell order wants limit price or higher
    // if market price is 0 then order always can match
    return ( (market == 0) || (market >= limit) );
}

// --------------------------------
// II. limit tree implementation where 1 node is Limit struct
// --------------------------------

template<Side side>
struct LimitTree {
    
    PriceLimitMap limitmap;
    Price last_best_price = 0;
    Count count = 0;
    Volume volume = 0;

    Limit* root = nullptr;
    Limit* best = nullptr;
    
    // place a limit order in the tree
    void placeLimit(Order* order) {
        // if there are no orders in the tree currently:
        if (limitmap.count(order->price) == 0) {
            order->limit = new Limit(order);
            // 
            BinarySearchTree::insert(
                // from Limit** (which is &root) to BST::Node<Price>**.
                // reinterpret_cast converts given pointer into another type
                // and also converts the type of variable pointed by the given pointer
                reinterpret_cast<BinarySearchTree::Node<Price>**>(&root),
                // 
                static_cast<BinarySearchTree::Node<Price>*>(order->limit)
            );
            set_best<side>(&best, order->limit);
            
        }
    }
};
}