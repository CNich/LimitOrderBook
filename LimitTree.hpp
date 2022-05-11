#ifndef LIMITTREE_HPP
#define LIMITTREE_HPP

#include "binary_search_tree.hpp"
#include "LOB_buildingBlocks.hpp"
#include <unordered_map>
//#include <boost/container/flat_map>
//using ska::flat_hash_map; // throws weird ass error 


namespace LOB {

typedef std::unordered_map<Price, Limit*> PriceLimitMap;
//typedef boost::intrusive::flat_map<Price, Limit*> PriceLimitMap;

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





// I.c. find best price in the binary tree

template<Side side>
void find_best(Limit** best);

template<>
void find_best<Side::Buy>(Limit** highest_buy) {
    if ((*highest_buy)->parent == nullptr) {
        *highest_buy = static_cast<Limit*>((*highest_buy)->left);
    } else {
        *highest_buy = static_cast<Limit*>((*highest_buy)->parent);
    }
    if (*highest_buy != nullptr) {
        *highest_buy = static_cast<Limit*>(BinarySearchTree::max(*highest_buy));
    }
}
template<>
void find_best<Side::Sell>(Limit** lowest_sell) {
    if ((*lowest_sell)->parent == nullptr) {
        *lowest_sell = static_cast<Limit*>((*lowest_sell)->right);
    } else {
        *lowest_sell = static_cast<Limit*>((*lowest_sell)->parent);
    }
    if (*lowest_sell != nullptr) {
        *lowest_sell = static_cast<Limit*>(BinarySearchTree::min(*lowest_sell));
    }
}

/*
template<>
void set_best<Side::Sell>(Limit** lowest_sell, Limit* limit) {
    if (*lowest_sell == nullptr || limit->key < (*lowest_sell)->key) {
        *lowest_sell = limit;
    }
}
*/

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
        // if given key is not found in the map
        // map.count(key) returns 1 if key is found, 0 otherwise
        // if given price is not found in the map
        if (limitmap.count(order->price) == 0) {
            order->limit = new Limit(order);
            // 
            BinarySearchTree::insert(
                // from Limit** (which is &root) to BST::Node<Price>**.
                // reinterpret_cast converts given pointer into another type
                // and also converts the type of variable pointed by the given pointer
                reinterpret_cast<BinarySearchTree::Node<Price>**>(&root),
                // make sure order->limit is pointer to node
                static_cast<BinarySearchTree::Node<Price>*>(order->limit)
            );
            set_best<side>(&best, order->limit);
            limitmap.emplace(order->limit->key, order->limit);
        } else { // if, in the map, the price of the order is found
            order->limit = limitmap.at(order->price);
            ++order->limit->count;
            
            order->limit->volume += order->quantity;
            DoublyLinkedList::append(
                reinterpret_cast<DoublyLinkedList::Node**>(&order->limit->order_tail),
                static_cast<DoublyLinkedList::Node*>(order)
            );
        }
        // updating variables for whole tree
        ++count;
        volume += order->quantity;
        if (best != nullptr) {
            last_best_price = best->key;
        }
    }

    // clearing the unordered map of <Price, Limit*>
    void clear() {
        // https://www.cplusplus.com/reference/unordered_map/unordered_map/begin/
        for(auto item = limitmap.begin(); item != limitmap.end(); item++) {
            delete item->second;
        }
        // unordered map destructor called, size becomes 0
        limitmap.clear();

        // reset 
        root = nullptr;
        best = nullptr;
        count = 0;
        volume = 0;
    }

    // find best price in the tree

    // cancel order
    void cancel(Order* order) {
        auto _limit = order->limit;

        // if input order is the only order in the tree
        if(order->prev == nullptr && order->next == nullptr) {
            // remove the node from tree
            BinarySearchTree::remove(
                reinterpret_cast<BinarySearchTree::Node<Price>**>(&root),
                static_cast<BinarySearchTree::Node<Price>*>(_limit)
            );
            // canceled order might be the best price,
            // so replace it if it's the case
            if (best == _limit){
                // find best price after taking out the canceled order
                find_best<side>(&best);
            }
            // erase element in map
            limitmap.erase(_limit->key);
            delete _limit;
        } else {
            --_limit->count;
            _limit->volume -= order->quantity;
            // remove order from dll
            DoublyLinkedList::remove(
                reinterpret_cast<DoublyLinkedList::Node**>(&_limit->order_head),
                reinterpret_cast<DoublyLinkedList::Node**>(&_limit->order_tail),
                static_cast<DoublyLinkedList::Node*>(order)
            );
        }

        --count;
        volume -= order->quantity;
        if (best != nullptr) {
            last_best_price = best->key;
        }

    }

    template<typename Callback>
    void market(Order* order, Callback did_fill) {
        while (best != nullptr && can_match<side>(best->key, order->price)) {
            auto match = best->order_head;
            // if best price order quantity >= input order quantity
            if (match->quantity >= order->quantity) {
                if (match->quantity == order->quantity) {
                    cancel(match);
                    did_fill(match->uid);
                } else {
                    match->quantity -= order->quantity;
                    match->limit->volume -= order->quantity;
                    volume -= order->quantity;
                }
                order->quantity = 0;
                return;
            }
            order->quantity -= match->quantity;
            cancel(match);
            did_fill(match->uid);
        }
    }

    inline Volume volume_at(Price price) const {
        if (limitmap.count(price)) return limitmap.at(price)->volume;
        return 0;
    }
    inline Count count_at(Price price) const {
        if (limitmap.count(price)) return limitmap.at(price)->count;
        return 0;
    }

};
}

#endif