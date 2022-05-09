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
};

}

#endif