#ifndef LOB_BUILDINGBLOCKS_HPP
#define LOB_BUILDINGBLOCKS_HPP
// limit tree implementation
#include <vector>
#include "doubly_linked_list.hpp"
#include "binary_search_tree.hpp"
#include <cstdint>


namespace LOB {

enum class Side : bool {Sell = false, Buy = true};
Side operator!(Side side) {
    return static_cast<Side>(!static_cast<bool>(side));
}

// -----------------------------------------
// basic building blocks of the limit orderbook
// -----------------------------------------

typedef uint64_t UID; // unique identifier of the order
typedef uint32_t Quantity; // quantity of ordered equities
typedef uint64_t Price; // at what price
//typedef int Price;
typedef uint32_t Count; // number of orders
typedef uint64_t Volume; // 

struct Limit;

struct Order : DoublyLinkedList::Node {
    const UID uid = 0;
    const Side side = Side::Sell;
    Quantity quantity = 0;
    const Price price = 0;
    Limit* limit = nullptr;

    // constructor:
    Order() : DoublyLinkedList::Node() { }
    Order(UID _uid, Side _side, Quantity _quantity, Price _price) :
        DoublyLinkedList::Node(),
        uid(_uid),
        side(_side),
        quantity(_quantity),
        price(_price) {}
};

struct Limit : BinarySearchTree::Node<Price> {
    Count count = 0;
    Volume volume = 0;
    Order* order_head = nullptr;
    Order* order_tail = nullptr;
    
    Limit() : BinarySearchTree::Node<Price>() {}

    explicit Limit(Order* order) :
    BinarySearchTree::Node<Price>(order->price),
    count(1),
    volume(order->quantity),
    order_head(order),
    order_tail(order){}
};

}

#endif