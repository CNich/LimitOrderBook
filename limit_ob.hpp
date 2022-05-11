#ifndef LIMIT_OB_HPP
#define LIMIT_OB_HPP

#include <unordered_map>
#include <tuple>
#include <string>
#include "LimitTree.hpp"
namespace LOB {

typedef std::unordered_map<UID, Order> uidOrdermap;

class LimitOrderBook {
    LimitTree<Side::Sell> selltree;
    LimitTree<Side::Buy> buytree;
    uidOrdermap ordermap;

    public:
        // init line:
        LimitOrderBook() : selltree(), buytree(), ordermap() {}
        inline void clear() {
            // implemented in LimitTree.hpp:
            selltree.clear();
            buytree.clear();

            // implemented in std::unordered_map
            ordermap.clear();
        }

        inline void limit_buy(UID order_id, Quantity quantity, Price price) {
            ordermap.emplace(std::piecewise_construct,
                // piecewise_construct is passed as 1st arg to construct a pair obj
                // https://stackoverflow.com/questions/6162201/c11-use-case-for-piecewise-construct-of-pair-and-tuple
                std::forward_as_tuple(order_id),
                std::forward_as_tuple(order_id, Side::Buy, quantity, price)
            );
            if (selltree.best != nullptr && price >= selltree.best->key) {
                selltree.market(&ordermap.at(order_id), [&](UID uid) {
                    ordermap.erase(uid);
                });
                if (ordermap.at(order_id).quantity == 0) {
                    ordermap.erase(order_id);
                    return;
                }
            }
            buytree.placeLimit(&ordermap.at(order_id));
        }

        inline void limit_sell(UID order_id, Quantity quantity, Price price) {
            ordermap.emplace(std::piecewise_construct,
                // piecewise_construct is passed as 1st arg to construct a pair obj
                // https://stackoverflow.com/questions/6162201/c11-use-case-for-piecewise-construct-of-pair-and-tuple
                std::forward_as_tuple(order_id),
                std::forward_as_tuple(order_id, Side::Sell, quantity, price)
            );
            if (buytree.best != nullptr && price <= buytree.best->key) {
                buytree.market(&ordermap.at(order_id), [&](UID uid) {
                    ordermap.erase(uid);
                });
                if (ordermap.at(order_id).quantity == 0) {
                    ordermap.erase(order_id);
                    return;
                }
            }
            selltree.placeLimit(&ordermap.at(order_id));
        }

        inline void limit(Side side, UID order_id, Quantity quantity, Price price) {
            switch (side) {
                case Side::Sell:
                    // std::cout<<"Side: Sell, "<<"UID: "<<order_id<<", "<<"Quantity: "<<quantity<<", "<<"Price: "<<price<<"\n";
                    return limit_sell(order_id, quantity, price);
                case Side::Buy:
                    //std::cout<<"Side: Buy, "<<"UID: "<<order_id<<", "<<"Quantity: "<<quantity<<", "<<"Price: "<<price<<"\n";
                    return limit_buy(order_id, quantity, price);
            }
        }
            

        inline bool has(UID order_id) const {
            return ordermap.count(order_id);
        }

        inline const Order& get(UID order_id) {
            return ordermap.at(order_id);
        }


        inline void cancel(UID id) {
            auto order = &ordermap.at(id);
            switch (order->side) {
                case Side::Sell: {
                    selltree.cancel(order);
                    break;
                }
                case Side::Buy: {
                    buytree.cancel(order);
                    break;
                }
            }
            ordermap.erase(id);
        }

        // to add: market orders, market sell buy and 
        // some function that returns best buy/sell price
        
        // market orders:
        // sell:
        inline void market_sell(UID id, Quantity q) {
            Order order{id, Side::Sell, q, 0}; // free sell order
            // lambda
            buytree.market(&order, [&](UID uid){ordermap.erase(uid);});
        }
        inline void market_buy(UID id, Quantity q) {
            Order order{id, Side::Buy, q, 0};
            selltree.market(&order, [&](UID id){ordermap.erase(id);});
        }
        inline void market(Side side, UID id, Quantity q) {
            switch (side) {
                case Side::Buy: {market_buy(id, q);};
                case Side::Sell: {market_sell(id, q);};
            }
        }


        // get best buy/sell prices
        inline Price best_sell() const {
            if (selltree.best == nullptr) {
                return 0;
            }
            return selltree.best->key;// cuz comparable key of tree is price
        }
        inline Price best_buy() const {
            if (buytree.best == nullptr) {
                return 0;
            }
            return buytree.best->key;
        }
        inline Price best(Side side) const {
            switch (side) {
                case Side::Sell: {return best_sell();};
                case Side::Buy: {return best_buy();};
            }
        }
};

}

#endif