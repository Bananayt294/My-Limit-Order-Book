#include "limit.hpp"
#include "book.hpp"
#include "order.hpp"
#include "order_pool.hpp"
#include "iostream"
#include <algorithm>
#include <random>
#include <iterator>
#include <vector>
#include "order_pool.hpp"

book::book() : buytree{nullptr}, selltree{nullptr}, highestbuy{nullptr}, lowestsell{nullptr}, stopbuytree{nullptr}, stopselltree{nullptr}, order_allocator{new order_pool()} {}

book::~book() {
    for (auto& [id, order] : order_map) {
        order_allocator->release(order);
    }
    order_map.clear();

    for (auto& [limitPrice, limit] : limitbuy_map) {
        delete limit;
    }
    limitbuy_map.clear();

    for (auto& [limitPrice, limit] : limitsell_map) {
        delete limit;
    }
    limitsell_map.clear();

    for (auto& [stopPrice, stopLevel] : stopmap) {
        delete stopLevel;
    }
    stopmap.clear();
}

void book::AddStopOrder(int orderid , bool buyorsell , int shares , int stopPrice){
    int executedOrdersCount = 0;
    int AVLTreeBalanceCount = 0;

    shares = stopOrderAsMarketOrder(orderid , buyorsell , shares , stopPrice);

    if (shares != 0){
        order* neworder = order_allocator->allocate(orderid , buyorsell , shares , 0);
        order_map.emplace(orderid , neworder);
        if (stopmap.find(stopPrice) == stopmap.end()){
            addLimit(stopPrice , buyorsell);
        }
        stopmap.at(stopPrice) -> order_append(neworder);
    }
}

void book::CancelStopLimitOrder(int orderId){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderId);
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            Order -> get_parent_limit() -> deleteLimit(Order -> get_parent_limit());
        }
        deleteFromOrderMap(Order);
        order_allocator->release(Order);
    };
};

int book::stopOrderAsMarketOrder(int orderid , bool buyorsell , int shares , int stopPrice){
    if (buyorsell && lowestsell != nullptr && stopPrice <= lowestsell -> get_limitPrice()){
        MarketOrderHelper(orderid , buyorsell , shares);
        return 0;
    }else if (!buyorsell && highestbuy != nullptr && stopPrice >= highestbuy -> get_limitPrice()){
        MarketOrderHelper(orderid , buyorsell , shares);
        return 0;
    }
    return shares;
}

void book::CancelStopOrder(int orderid){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderid);
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            Order -> get_parent_limit() -> deleteLimit(Order -> get_parent_limit());
        }
        deleteFromOrderMap(Order);
        order_allocator->release(Order);
    }
}

void book::ModifyStopLimitOrder(int orderId, int newShares, int newLimitPrice, int newStopPrice){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderId);
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            Order -> get_parent_limit() -> deleteLimit(Order -> get_parent_limit());
        }
        Order -> modifyorder(newShares , newLimitPrice);
        if (stopmap.find(newStopPrice) == stopmap.end()){
            addLimit(newStopPrice , Order -> get_buyorsell());
        }
        stopmap.at(newStopPrice) -> order_append(Order);
    }
}

void book::ModifyStopOrder(int orderid , bool buyorsell , int shares , int stopPrice){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderid);
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            Order -> get_parent_limit() -> deleteLimit(Order -> get_parent_limit());
        }
        Order -> setShares(shares);
        if (stopmap.find(stopPrice) == stopmap.end()){
            addLimit(stopPrice , buyorsell);
        }
        stopmap.at(stopPrice) -> order_append(Order);
    }
}

void book::ModifyLimitOrder(int orderId, int newShares, int newLimit){
    auto& order = order_map.at(orderId);
    if (order != nullptr){
        order -> cancel();
        if (order -> get_parent_limit() -> get_size() == 0){
            order -> get_parent_limit() -> deleteLimit(order -> get_parent_limit());
        }
        auto& limit_map = order -> get_buyorsell() ? limitbuy_map : limitsell_map;
        order -> modifyorder(newShares , newLimit);
        if (limit_map.find(newLimit) == limit_map.end()){
            addLimit(newLimit , order -> get_buyorsell());
        }
        limit_map.at(newLimit) -> order_append(order);
    }
};

limit* book::getLowestSell() const{
    return lowestsell;
}

limit* book::getHighestBuy() const{
    return highestbuy;
}



int book::getLimitHeight(limit* node)
{
    if(node == nullptr)
        return 0;

    return 1 + std::max(getLimitHeight(node->get_leftchild()),
                        getLimitHeight(node->get_rightchild()));
}

int book::getLeftSideHeight(limit* Limit){
    if (Limit == nullptr){
        return 0;
    }
    return getLimitHeight(Limit -> get_leftchild());
}

int book::getRightSideHeight(limit* Limit){
    if (Limit == nullptr){
        return 0;
    }
    return getLimitHeight(Limit -> get_rightchild());
}

limit* book::insert(limit* root , limit* newlimit , limit* parent){
    if (root == nullptr){
        newlimit -> setParent(parent);
        return newlimit;
    }
    if (newlimit -> get_limitPrice() < root -> get_limitPrice()){
        root -> setleftchild(insert(root->get_leftchild() , newlimit , root));

    }
    else if (newlimit -> get_limitPrice() > root -> get_limitPrice()){
        root -> setrightchild(insert(root->get_rightchild() , newlimit , root));
    }
    root = balanceTree(root);
    return root;
}//fixed Insert

void book::addLimit(int limit_price , bool buyorsell){
    auto& Limitmap = buyorsell ? limitbuy_map : limitsell_map;
    auto& tree = buyorsell ? buytree : selltree;
    auto& bookedge = buyorsell ? highestbuy : lowestsell;

   limit* newlimit = new limit(limit_price , buyorsell);
   Limitmap.emplace(limit_price , newlimit);
   if (tree == nullptr){
    tree = newlimit;
    bookedge = newlimit;
   }else{
    limit* tree = insert(tree , newlimit);
    updateBookEdgeInsert(newlimit);
   }
}

void book::updateBookEdgeInsert(limit* newlimit){
    auto& tree = newlimit -> getbuyorsell() ? buytree : selltree;
    if(tree == buytree){
        if(newlimit -> get_limitPrice() > highestbuy -> get_limitPrice()){
            highestbuy = newlimit;
        }
    }else if (tree == selltree){
        if(newlimit -> get_limitPrice() < lowestsell -> get_limitPrice()){
            lowestsell = newlimit;
        };
    };
};
//MUST UPDATE ADD LIMIT ORDER FUNCTION
void book::AddLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice)
{
    auto AVLTreeBalanceCount = 0;
    // Account for order being executed immediately
    shares = LimitOrderAsMarketOrder(orderId, buyOrSell, shares, limitPrice);
    
    if (shares != 0)
    {
        order* newOrder = order_allocator->allocate(orderId, buyOrSell, shares, limitPrice);
        order_map.emplace(orderId, newOrder);

        auto& limitMap = buyOrSell ? limitbuy_map : limitsell_map;

        if (limitMap.find(limitPrice) == limitMap.end())
        {
            addLimit(limitPrice, newOrder->get_buyorsell());
        }
        limitMap.at(limitPrice)->order_append(newOrder);
        // limitOrders.insert(newOrder);
    }
    else
    {
        executeStopOrders(buyOrSell);
    }
}

void book::AddStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice)
{
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    // Account for stop limit order being executed immediately
    shares = StopLimitOrderAsLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
    
    if (shares != 0)
    {
        order* newOrder = order_allocator->allocate(orderId, buyOrSell, shares, limitPrice);
        order_map.emplace(orderId, newOrder);

        if (stopmap.find(stopPrice) == stopmap.end())
        {
            addLimit(stopPrice, newOrder->get_buyorsell());
        }
        stopmap.at(stopPrice)->order_append(newOrder);
        // stopLimitOrders.insert(newOrder);
    }
}

void book::executeStopOrders(bool buyOrSell)
{
    auto& bookEdge = buyOrSell ? lowestsell : highestbuy;
    while (bookEdge != nullptr)
    {
        order* headOrder = bookEdge->get_headOrder();
        if (headOrder->get_buyorsell() == buyOrSell)
        {
            // stopLimitOrders.erase(headOrder);
            stopLimitOrderToLimitOrder(headOrder, buyOrSell);
        }
        else
        {
            break;
        }
    }
}

void book::stopLimitOrderToLimitOrder(order* stopLimitOrder, bool buyOrSell)
{
    int orderId = stopLimitOrder->get_idNumber();
    int shares = stopLimitOrder->getshares();
    int limitPrice = stopLimitOrder->get_Limit();
    CancelStopOrder(orderId);
    AddLimitOrder(orderId, buyOrSell, shares, limitPrice);
};

void book::CancelLimitOrder(int orderId)
{
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderId);
    if (Order != nullptr)
    {
        Order->cancel();
        if (Order->get_parent_limit()->get_size() == 0)
        {
            Order->get_parent_limit()->deleteLimit(Order->get_parent_limit());
        }
        deleteFromOrderMap(Order);
        order_allocator->release(Order);
    }
}

int book::StopLimitOrderAsLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice)
{
    if (buyOrSell && lowestsell != nullptr && stopPrice <= lowestsell->get_limitPrice())
    {
        AddLimitOrder(orderId, true, shares, limitPrice);
        return 0;
    } else if (!buyOrSell && highestbuy != nullptr && stopPrice >= highestbuy->get_limitPrice())
    {
        AddLimitOrder(orderId, false, shares, limitPrice);
        return 0;
    }
    return shares;
}

int book::LimitOrderAsMarketOrder(int orderId, bool buyOrSell, int shares, int limitPrice)
{
    if (buyOrSell)
    {
        while (lowestsell != nullptr && shares != 0 && lowestsell->get_limitPrice() <= limitPrice)
        {
            if (shares <= lowestsell->get_totalshares())
            {
                MarketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            } else {
                shares -= lowestsell->get_totalshares();
                MarketOrderHelper(orderId, buyOrSell, lowestsell->get_totalshares());
            }
        }
        return shares;
    } else {
        while (highestbuy != nullptr && shares != 0 && highestbuy->get_limitPrice() >= limitPrice)
        {
            if (shares <= highestbuy->get_totalshares())
            {
                MarketOrderHelper(orderId, buyOrSell, shares);
                return 0;
            } else {
                shares -= highestbuy->get_totalshares();
                MarketOrderHelper(orderId, buyOrSell, highestbuy->get_totalshares());
            }
        }
        return shares;
    }
}

void book::MarketOrderHelper(int orderid , bool buyorsell,int shares){
    auto& bookedge = buyorsell ? lowestsell : highestbuy;
    auto executedOrdersCount = 0;

    while(bookedge!= nullptr && bookedge -> get_headOrder() -> getshares() <= shares){
        order* headorder = bookedge -> get_headOrder();
        shares -= headorder -> getshares();
        headorder -> execute();
        if (bookedge -> get_size() == 0){
            bookedge -> deleteLimit(bookedge);   
        }
        deleteFromOrderMap(headorder);
        order_allocator->release(headorder);
        executedOrdersCount++;
}   if (bookedge != nullptr && shares != 0){
    bookedge -> get_headOrder() -> partiallyFillOrder(shares);
    executedOrdersCount++;
    }
}

void book::deleteFromOrderMap(order* Order){
    auto it = order_map.find(Order -> get_idNumber());
    if (it != order_map.end()){
        order_map.erase(it);
    }
};

std::vector<int> book::PreorderTraversal(limit* root) {
    auto result = std::vector<int>();
    if (root != nullptr) {
        PreorderHelper(root, result);
    }
    return result;
};

void book::PreorderHelper(limit* root,std::vector<int>& result){
    if (root != nullptr){
        result.push_back(root -> get_limitPrice());
        PreorderHelper(root -> get_leftchild(), result);
        PreorderHelper(root -> get_rightchild(), result);
    }
};

std::vector<int> book::PostorderTraversal(limit* root) {
    auto result = std::vector<int>();
    if (root != nullptr) {
        PostorderHelper(root, result);
    }
    return result;
};

void book::PostorderHelper(limit* root,std::vector<int>& result){
    if (root != nullptr){
        PostorderHelper(root -> get_leftchild(), result);
        PostorderHelper(root -> get_rightchild(), result);
        result.push_back(root -> get_limitPrice());
    }
};

std::vector<int> book::InorderTraversal(limit* root) {
    auto result = std::vector<int>();
    if (root != nullptr) {
        inOrderTreeHelper(root, result);
    }
    return result;
};

std::vector<int> book::inOrderTreeHelper(limit* root,std::vector<int>& result){
    if (root != nullptr){
        inOrderTreeHelper(root -> get_leftchild(), result);
        result.push_back(root -> get_limitPrice());
        inOrderTreeHelper(root -> get_rightchild(), result);
    }
    return result;
}

limit* book::balanceTree(limit* Limit){
    int b_factor = get_b(Limit);
    if (b_factor > 1){
        if (get_b(Limit -> get_leftchild()) >= 0){
            return LL_rebalance(Limit);
        }else{
            return LR_rebalance(Limit);
        }
    }else if (b_factor < -1){
        if (get_b(Limit -> get_rightchild()) <= 0){
            return RR_rebalance(Limit);
        }else{
            return RL_rebalance(Limit);
        }
    }
    return Limit;
}

int book::get_b(limit* Limit){
    int left = getLeftSideHeight(Limit);
    int right = getRightSideHeight(Limit);
    int b_factor = (left - right);
    return b_factor;
};

limit* book::LL_rebalance(limit* Limit){
    limit* newparent = Limit -> get_leftchild();
    if (Limit -> get_parent() != nullptr){
        newparent -> setParent(Limit -> get_parent());
        if (Limit -> get_parent() -> get_leftchild() == Limit){
            Limit -> get_parent() -> setleftchild(newparent);
        }
         else if (Limit -> get_parent() -> get_rightchild() == Limit){
        Limit -> get_parent() -> setrightchild(newparent);
        }
    }else{
        newparent -> setParent(nullptr);
        auto& tree = Limit -> getbuyorsell() ? buytree : selltree;
        tree = newparent;
    }if (newparent -> get_rightchild() != nullptr){
        newparent -> get_rightchild() -> setParent(Limit);
    }
    Limit -> setleftchild(newparent -> get_rightchild());
    newparent -> setrightchild(Limit);
    Limit -> setParent(newparent);
    return newparent;
};//fixed rebalance
limit* book::RR_rebalance(limit* Limit){
    limit* newparent = Limit -> get_rightchild();
    if (Limit -> get_parent() != nullptr){
        newparent -> setParent(Limit -> get_parent());
        if (Limit -> get_parent() -> get_leftchild() == Limit){
            Limit -> get_parent() -> setleftchild(newparent);
        }
         else if (Limit -> get_parent() -> get_rightchild() == Limit){
            Limit -> get_parent() -> setrightchild(newparent);
        }
    }else{
        newparent -> setParent(nullptr);
        auto& tree = Limit -> getbuyorsell() ? buytree : selltree;
        tree = newparent;
    }
    Limit -> setrightchild(newparent -> get_leftchild());
    if (newparent -> get_leftchild() != nullptr){
        newparent -> get_leftchild() -> setParent(Limit);
    }
    
    newparent -> setleftchild(Limit);
    Limit -> setParent(newparent);
    return newparent;
}//fixed rebalance

limit* book::LR_rebalance(limit* Limit){
    RR_rebalance(Limit -> get_leftchild());
    return LL_rebalance(Limit);
}

limit* book::RL_rebalance(limit* Limit){
    LL_rebalance(Limit -> get_rightchild());
    return RR_rebalance(Limit);
}

