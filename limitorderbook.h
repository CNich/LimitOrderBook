#ifndef LIMIT_ORDERBOOK_H
#define LIMIT_ORDERBOOK_H

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

// aim is to implement binary tree of limit objects sorted by limit price.
// each Limit object is a doubly linked list of order objects

#endif
