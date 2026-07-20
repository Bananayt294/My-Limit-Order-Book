#ifndef BOOK_HPP
#define BOOK_HPP

#include <random>
#include <unordered_map>
#include <vector>

class order_pool;
class limit;
class order;

class book{
    private:
    limit* buytree;
    limit* selltree;
    limit* highestbuy;
    limit* lowestsell;
    limit* stopbuytree;
    limit* stopselltree;
    limit* higheststopbuy;
    limit* loweststopsell;

    std::unordered_map<int, order*> order_map;
    order_pool* order_allocator;
    std::unordered_map<int, limit*> limitbuy_map;
    std::unordered_map<int, limit*> limitsell_map;
    std::unordered_map<int, limit*> stopmap;

    limit* RL_rebalance(limit* Limit);
    limit* RR_rebalance(limit* Limit);
    limit* LR_rebalance(limit* Limit);
    limit* LL_rebalance(limit* Limit);
    
    public:
    limit* insert(limit* root, limit* newlimit,limit* parent = nullptr);
    void addLimit(int limit_price , bool buyorsell);
    void updateBookEdgeInsert(limit* newlimit);

    ~book();
    book();

    void ModifyLimitOrder(int orderId, int newShares, int newLimit);
    void marketOrder(int orderId, bool buyOrSell, int shares);
    void AddLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice);
    void AddStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice);
    void CancelLimitOrder(int orderId);
    int LimitOrderAsMarketOrder(int orderid , bool buyorsell , int shares , int limit_price);
    int StopLimitOrderAsLimitOrder(int orderid , bool buyorsell , int shares , int limit_price , int stop_price);
    void AddStopOrder(int orderid , bool buyorsell , int shares , int stop_price);
    void ModifyStopOrder(int orderid , bool buyorsell , int shares , int stop_price);
    void ModifyStopOrder(int orderId, int newShares, int newStopPrice);
    void CancelStopOrder(int orderid);
    void CancelStopLimitOrder(int orderId);
    void ModifyStopLimitOrder(int orderId, int newShares, int newLimitPrice, int newStopPrice);
    void executeStopOrders(bool buyorsell);

    void MarketOrderHelper(int orderid , bool buyorsell , int shares);
    limit* getLowestSell() const;
    limit* getHighestBuy() const;
    limit* getHighestStopSell() const;
    limit* getLowestStopBuy() const;
    order* getRandomOrder(int key, std::mt19937 gen) const;

    
    limit* balanceTree(limit* Limit);
    void changeBookRoots(limit* Limit);
    int getLimitHeight(limit* node);
    int get_b(limit* Limit);
    int getRightSideHeight(limit* Limit);
    int getLeftSideHeight(limit* Limit);
 

    void deleteFromOrderMap(order* Order);
    void partiallyFillOrder(int shares);

    std::vector<int> PreorderTraversal(limit* root);
    void PreorderHelper(limit* root,std::vector<int>& result);
    std::vector<int> InorderTraversal(limit* root);
    std::vector<int> inOrderTreeHelper(limit* root,std::vector<int>& result);
    std::vector<int> PostorderTraversal(limit* root);
    void PostorderHelper(limit* root,std::vector<int>& result);
    void stopLimitOrderToLimitOrder(order* stopLimitOrder, bool buyOrSell);
    

    int stopOrderAsMarketOrder(int orderid , bool buyorsell , int shares , int stopPrice);
    void stopOrderHelper(int orderid , bool buyorsell , int shares);
};

#endif
