#ifndef LIMIT_ORDERBOOK_H
#define LIMIT_ORDERBOOK_H

#include <vector>

using namespace boost::intrusive;
// doubly linked list: 
// https://www.boost.org/doc/libs/1_35_0/doc/html/intrusive/list.html
#include <boost/intrusive/list.hpp>
// binary search tree:
// https://www.boost.org/doc/libs/1_66_0/doc/html/boost/intrusive/bstree.html
#include <boost/intrusive/bstree.hpp>

/*
class Order {
    int idNumber;
    bool buyOrSell;
    int shares;
    int limit;
    int entryTime;
    int eventTime;
    Order *nextOrder;
    Order *prevOrder;
    Limit *parentLimit;
};

class Limit { // representing a single limit price
  int limitPrice;
  int size;
  int totalVolume;
  Limit* parent;
  Limit* leftChild;
  Limit* rightChild;
  Order* headOrder;
  Order* tailOrder;
};

class Book {
  Limit* buyTree;
  Limit* sellTree;
  Limit* lowestSell;
  Limit* highestBuy;
};
*/

enum class Side : bool {Sell = false, Buy = true};

// typedef lets you know the purpose of a variable since 
// words like double or int don't have any contexts
typedef uint64_t UID;
typedef uint32_t Quantity;
typedef uint64_t Price;
typedef uint32_t Count;
typedef uint64_t Volume;

struct Order : list::node {

};

struct Limit : bstree::node<Price>() {
  Count count = 0;
  Volume volume = 0;
  Order* order_head = nullptr; // first order in queue (first to execute)
  Order* order_tail = nullptr; // last order in Q
};

// aim is to implement binary tree of limit objects sorted by limit price.
// each Limit object is a doubly linked list of order objects

#endif
