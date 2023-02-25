#ifndef LIMIT_OB_HPP
#define LIMIT_OB_HPP

#include <unordered_map>
#include <tuple>
#include <string.h>
#include "LimitTree.hpp"

#define MAX_COLUMNS 32
#define MAX_LINE_LEN 1024

namespace LOB {

typedef std::unordered_map<UID, Order> uidOrdermap;
const Quantity MAX_QUANTITY = 18474407370916;
const int fac = 1000;

class LimitOrderBook {
    LimitTree<Side::Sell> selltree;
    LimitTree<Side::Buy> buytree;
    uidOrdermap ordermap;
    uint64_t count = 0;
    uint64_t last_ts = 0;
    uint64_t min_ts = 0;
    uint64_t last_update_id = 0;

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

        inline void limit_buy_L2(UID uid, Quantity quantity, Price price) {
            if (quantity > 0){
                if (buytree.limitmap.count(price) > 0){
                    buytree.limitmap.at(price)->order_head->quantity = static_cast<Volume>(quantity);
                }
                else{
                    // check if bid crosses the spread. If it doesn't, then
                    // there is nothing to do. If it does, then we first place
                    // a limit_buy with MAX_QUANTITY to clear the book until
                    // our price, then switch the quantity to the actual order.
                    if(best_sell() < price){
                        // clear asks up to price
                        limit_buy(uid, MAX_QUANTITY, price);
                        // alter quantity
                        cancel(uid);
                        limit_buy(uid, quantity, price);
                    }
                    else{
                        limit_buy(uid, quantity, price);
                    }
                }
            }
            else{
                if (buytree.limitmap.count(price) > 0){
                    limit_buy(uid, 0, price);
                }
            }
        }

        inline void limit_sell_L2(UID uid, Quantity quantity, Price price) {
            if (quantity > 0){
                if (selltree.count_at(price) > 0){
                    selltree.limitmap.at(price)->order_head->quantity = static_cast<Volume>(quantity);
                    //selltree.limitmap.at(price)->volume = quantity;
                }
                else{
                    // check if ask crosses the spread. If it doesn't, then
                    // there is nothing to do. If it does, then we first place
                    // a limit_sell with MAX_QUANTITY to clear the book until
                    // our price, then switch the quantity to the actual order.
                    if(best_buy() > price){
                        limit_sell(uid, MAX_QUANTITY, price);
                        // for some reason, we cannot do
                        // selltree.limitmap.at(price)->volume = quantity;
                        // without getting an error...
                        cancel(uid);
                        limit_sell(uid, quantity, price);
                    }
                    else{
                        limit_sell(uid, quantity, price);
                    }
                }
            }
            else{
                if (selltree.limitmap.count(price) > 0){
                    limit_sell(uid, 0, price);
                }
            }
        }

        inline Quantity quantity_at(Side side, Price p){
            switch (side) {
                case Side::Sell:
                    return selltree.quantity_at(p);
                case Side::Buy:
                    return buytree.quantity_at(p);
            }
        }

        inline void print_sorted(Side side){
            switch (side) {
                case Side::Sell:
                    selltree.print_sorted();
                    break;
                case Side::Buy:
                    buytree.print_sorted();
                    break;
            }
        }

        inline void market_impact(Side side, double* facs, int L){
            switch (side) {
                case Side::Sell:
                    buytree.market_impact(facs, L);
                    break;
                case Side::Buy:
                    selltree.market_impact(facs, L);
                    break;
            }
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
                    return limit_sell(order_id, quantity, price);
                case Side::Buy:
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

        inline int parse_csv_line(char* line, char** values) {
            int column = 0;
            char* value = strtok(line, ",");
            while (value != NULL) {
                values[column++] = value;
                value = strtok(NULL, ",");
            }
            return column;
        }

        inline int set_min_ts(uint64_t ts){
            min_ts = ts;
        }

        inline int load_csv(int divisor, char *argv, int option, uint64_t max_ts) {
            std::cout<<"LOADING CSV best_buy: " << best_buy() << ", best_sell: " << best_sell() << "\n";
            int num_p = 1000000;
            int row_count =  135000000;
            char* values[MAX_COLUMNS];
            int num_columns;

            int i_symbol = 0;
            int i_timestamp = 1;
            int i_first_update_id = 2;
            int i_last_update_id = 3;
            int i_side = 4;
            int i_update_type = 5;
            int i_price = 6;
            int i_qty = 7;
            int i_pu = 8;

            uint64_t csv_ts = 0;
            int step = 2;
            int interval_ms = 1000;
            uint64_t ms_in_day = 1000*60*60;
            uint64_t num_rows = ms_in_day / (step * interval_ms);
            int j = 0;

            char* filename = "test_write.csv";
            char* column_names = "ts,best_bid,best_ask,b0005,b0010,b0015,b0020,b0025,b0030,b0035,b0040,b0045,b0050,a0005,a0010,a0015,a0020,a0025,a0030,a0035,a0040,a0045,a0050";

            double facs[10] = {0.0005, 0.001, 0.0015, 0.002, 0.0025, 0.003, 0.0035, 0.004, 0.0045,  0.005};
            int L = sizeof(facs)/sizeof(facs[0]);

            Volume c_vols[L];

            clock_t start = clock();

            // Open the file for reading
            FILE *fp = fopen(argv, "r");
            if (fp == NULL) {
                printf("Error: Unable to open file %s\n", argv);
                return 1;
            }

            // Read the file line by line
            char line[MAX_LINE_LEN];
            char ask[] = "a";
            uint64_t ts = 0;
            uint64_t prev_ts = 0;
            while (fgets(line, MAX_LINE_LEN, fp) && count < row_count / divisor) {
                // Remove the newline character from the end of the line
                line[strcspn(line, "\r\n")] = 0;
                num_columns = parse_csv_line(line, values);
                Price p = static_cast<Price>(strtod(values[i_price], NULL) * fac);
                Quantity q = static_cast<Price>(strtod(values[i_qty], NULL) * fac);

                //already in milliseconds
                ts = static_cast<uint64_t>(strtoul(values[i_timestamp], NULL, 10));
                UID uid = count;

                if(max_ts != 0 && ts > max_ts){
                    std::cout<<"breaking\n";
                    break;
                }

                prev_ts = ts;

                if(ts >= min_ts && last_ts <= ts){
                    if (values[i_side][0] == ask[0]){
                        limit_sell_L2(uid, q, p);
                    }
                    else{
                        limit_buy_L2(uid, q, p);
                    }
                    count++;
                }
            }

            // Close the file
            fclose(fp);
            clock_t end = clock();
            double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
            std::cout<<"Time taken to load " << count << " records: " << time_taken << "seconds\n";
            std::cout<<"=======================================\n";

            start = clock();
            market_impact(Side::Buy, facs, L);
            market_impact(Side::Sell, facs, L);
            end = clock();
            time_taken = (double)(end - start) / CLOCKS_PER_SEC;
            time_taken = (double)(end - start) / CLOCKS_PER_SEC;
            printf("Time for linked list market impacts: %f seconds\n", time_taken);
            std::cout<<"=======================================\n";
            std::cout<<"final prev_ts: " << prev_ts << "\n";
            std::cout<<"=======================================\n";

            return 0;
        }
};

}

#endif
