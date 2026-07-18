#include "unordered_map"
#include <random>
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

    void modifylimitorder(int idnumber,int newlimit,int newshares);
    void marketOrder(int orderId, bool buyOrSell, int shares);
    void addLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice);
    void addStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice);
    void cancelLimitOrder(int orderId);
    void modifyLimitOrder(int orderId, int newShares, int newLimit);
    int LimitOrderAsMarketOrder(int orderid , bool buyorsell , int shares , int limit_price);
    int stopLimitOrderAsLimitOrder(int orderid , bool buyorsell , int shares , int limit_price , int stop_price);
    void addStopOrder(int orderid , bool buyorsell , int shares , int stop_price);
    void modifyStopOrder(int orderid , bool buyorsell , int shares , int stop_price);
    void modifyStopOrder(int orderId, int newShares, int newStopPrice);
    void cancelStopOrder(int orderid);
    void cancelStopLimitOrder(int orderId);
    void modifyStopLimitOrder(int orderId, int newShares, int newLimitPrice, int newStopPrice);
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