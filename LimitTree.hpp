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


template<Side side>
void print_sorted_template(Limit* best);

template<>
void print_sorted_template<Side::Buy>(Limit* best) {
    SortedOrder* tmp = best->order_head->sorted_order;
    int i = 0;
    int num_errors = 0;
    Price p = tmp->order->price;
    while(tmp->prev != nullptr && i < 1000){
        std::cout<<"b " << i << " " << tmp->order->price << " " << tmp->order->quantity << "\n";
        tmp = reinterpret_cast<SortedOrder*>(tmp->prev);
        i++;
        if(i > 1){
            if(p < tmp->order->price){
                num_errors++;
            }
            p = tmp->order->price;
        }
    }
    std::cout<<tmp->order->price << " ";
    std::cout<<"\n";
    std::cout<<"num errors: " << num_errors << "\n";
    }

template<>
void print_sorted_template<Side::Sell>(Limit* best) {
    SortedOrder* tmp = best->order_tail->sorted_order;
    int i = 0;
    int num_errors = 0;
    Price p = tmp->order->price;
    while(tmp->next != nullptr && i < 1000){
        std::cout<<"s " << i << " " << tmp->order->price << " " << tmp->order->quantity << "\n";
        tmp = reinterpret_cast<SortedOrder*>(tmp->next);
        i++;
        if(i > 1){
            if(p > tmp->order->price){
                num_errors++;
            }
            p = tmp->order->price;
        }
    }
    std::cout<<tmp->order->price << " ";
    std::cout<<"\n";
    std::cout<<"num errors: " << num_errors << "\n";
    }

template<Side side>
void market_impact_calc(Order* best, Volume* c_vols, Price* p_arr, int L, int &num_nodes_visited);

template<>
void market_impact_calc<Side::Buy>(Order* best, Volume* c_vols, Price* p_arr, int L, int &num_nodes_visited){
    auto current = best->sorted_order;
    num_nodes_visited = 0;

    while(current->prev != nullptr && current->order->price >= p_arr[L-1]){
        auto v = current->order->quantity;
        for(unsigned int i=0; i < L; i++){
            if (current->order->price >= p_arr[i]){
                c_vols[i] += v;
            }
        }
        current = reinterpret_cast<SortedOrder*>(current->prev);
        num_nodes_visited++;
    }
}

template<>
void market_impact_calc<Side::Sell>(Order* best, Volume* c_vols, Price* p_arr, int L, int &num_nodes_visited){
    auto current = best->sorted_order;
    num_nodes_visited = 0;

    while(current->next != nullptr && current->order->price <= p_arr[L-1]){
        auto v = current->order->quantity;
        for(unsigned int i=0; i < L; i++){
            if (current->order->price <= p_arr[i]){
                c_vols[i] += v;
            }
        }
        current = reinterpret_cast<SortedOrder*>(current->next);
        num_nodes_visited++;
    }
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

    SortedOrder* sorted_head = nullptr;
    SortedOrder* sorted_tail = nullptr;

    int tmp_count = 0;

    // place a limit order in the tree
    void placeLimit(Order* order) {
        order->sorted_order = new SortedOrder(order);
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


            if(order->limit->parent != nullptr){
                if(order->limit->parent->key > order->price){
                    DoublyLinkedList::insert_before(
                        static_cast<DoublyLinkedList::Node*>(reinterpret_cast<Limit*>(reinterpret_cast<Limit*>(order->limit)->parent)->order_head->sorted_order),
                        static_cast<DoublyLinkedList::Node*>(order->sorted_order)
                    );
                }
                else if(order->limit->parent->key < order->price){
                    DoublyLinkedList::insert_after(
                        static_cast<DoublyLinkedList::Node*>(reinterpret_cast<Limit*>(reinterpret_cast<Limit*>(order->limit)->parent)->order_tail->sorted_order),
                        static_cast<DoublyLinkedList::Node*>(order->sorted_order)
                    );
                }



                else { // if, in the map, the price of the order is found
                    order->limit = limitmap.at(order->price);

                    // ++i increments and then returns the incremented value
                    // i++ increments i too but returns i before increment
                    ++order->limit->count;

                    order->limit->volume += order->quantity;

                    DoublyLinkedList::insert_after(
                        static_cast<DoublyLinkedList::Node*>(reinterpret_cast<Limit*>(order->limit)->order_tail->sorted_order),
                        static_cast<DoublyLinkedList::Node*>(order->sorted_order)
                    );

                    DoublyLinkedList::append(
                        reinterpret_cast<DoublyLinkedList::Node**>(&order->limit->order_tail),
                        static_cast<DoublyLinkedList::Node*>(order)
                    );
                }
            }
        }

        // updating variables for whole tree
        ++count;
        volume += order->quantity;
        if (best != nullptr) {
            last_best_price = best->key;
        }

        tmp_count++;
    }

    void inline print_sorted(){
        print_sorted_template<side>(best);
    }

    void inline print_triplet(Order* order){
        auto prev = reinterpret_cast<SortedOrder*>(order->sorted_order->prev);
        auto next = reinterpret_cast<SortedOrder*>(order->sorted_order->next);
        if(prev != nullptr && prev->order != nullptr){
            std::cout<<"prev " << prev->order->price<<"\n";
        }
        if(next != nullptr && next->order != nullptr){
            std::cout<<"next " << next->order->price<<"\n";
        }
        if(prev != nullptr && next != nullptr){
            std::cout<<  reinterpret_cast<SortedOrder*>(order->sorted_order->prev)->order->price << " " << order->price << " " << reinterpret_cast<SortedOrder*>(order->sorted_order->next)->order->price << "\n";
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
        DoublyLinkedList::headless_remove(
            static_cast<DoublyLinkedList::Node*>(order->sorted_order)
        );

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

    inline Quantity quantity_at(Price price) const {
        if (limitmap.count(price)) return limitmap.at(price)->order_head->quantity;
        return 0;
    }

    inline Volume volume_at(Price price) const {
        if (limitmap.count(price)) return limitmap.at(price)->volume;
        return 0;
    }

    inline Count count_at(Price price) const {
        if (limitmap.count(price)) return limitmap.at(price)->count;
        return 0;
    }

    inline void market_impact(Volume* c_vols, double* facs, int L){
        int num_nodes_visited = 0;
        //Volume c_vols[L];
        for(unsigned int i=0; i < L; i++){
            c_vols[i] = 0;
        }
        Order* order;
        char* pside;

        Price prices[L];
        switch (side) {
            case Side::Buy:
                for(unsigned int i=0; i < L; i++){
                    prices[i] = static_cast<Price>(best->key * (1 - facs[i]));
                }
                order = best->order_tail;
                pside = "buy";
                break;

            case Side::Sell:
                for(unsigned int i=0; i < L; i++){
                    prices[i] = static_cast<Price>(best->key * (1 + facs[i]));
                }
                order = best->order_head;
                pside = "sell";
                break;
            }
        market_impact_calc<side>(order, c_vols, prices, L, num_nodes_visited);

        //std::cout<<"best: " << best->key << "\n";
        //for(unsigned int i=0; i < L; i++){
        //    std::cout << "market " << pside << " liquidity at price " << prices[i] << " is volume " << c_vols[i] << "\n";
        //}
        //
        //std::cout << num_nodes_visited << " nodes visited.\n";
    }

};
}

#endif
