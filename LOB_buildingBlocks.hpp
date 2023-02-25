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
typedef uint64_t Quantity; // quantity of ordered equities
typedef uint64_t Price; // at what price
typedef uint64_t Count; // number of orders
typedef uint64_t Volume; // number of orders that was actually executed

struct Limit;
struct Order;

// Holds position of sorted orders
struct SortedOrder: DoublyLinkedList::Node {
    Order* order= nullptr;
    SortedOrder() : DoublyLinkedList::Node() { }
    SortedOrder(Order* _order) :
        DoublyLinkedList::Node(),
        order(_order) {}
};


// Order inherits from DLL::Node
struct Order : DoublyLinkedList::Node {
    const UID uid = 0;
    const Side side = Side::Sell;
    Quantity quantity = 0;
    const Price price = 0;
    Limit* limit = nullptr;
    SortedOrder* sorted_order = nullptr;

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

    // constructor
    Limit() : BinarySearchTree::Node<Price>() {}

    // prevent auto type conversion
    // for any func that receives Limit struct as
    // input, have to make sure that any other input
    // type raises error
    explicit Limit(Order* order) :
    BinarySearchTree::Node<Price>(order->price),
    count(1),
    volume(order->quantity),
    order_head(order),
    order_tail(order){}
};

}

#endif
