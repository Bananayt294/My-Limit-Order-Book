#include "limit.hpp"
#include "book.hpp"
#include "order.hpp"
#include "iostream"
#include <algorithm>
#include <random>
#include <iterator>
#include <vector>

book::book() : buytree{nullptr}, selltree{nullptr}, highestbuy{nullptr}, lowestsell{nullptr},stopbuytree{nullptr},stopselltree{nullptr} {};

book::~book() {
    for (auto& [id, order] : order_map) {
        delete order;
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

void book::addStopOrder(int orderid , bool buyorsell , int shares , int stopPrice){
    int executedOrdersCount = 0;
    int AVLTreeBalanceCount = 0;

    shares = stopOrderAsMarketOrder(orderid , buyorsell , shares , stopPrice);

    if (shares != 0){
        order* neworder = new order(orderid , buyorsell , shares , 0);
        order_map.emplace(orderid , neworder);
        if (stopmap.find(stopPrice) == stopmap.end()){
            addLimit(stopPrice , buyorsell);
        }
        stopmap.at(stopPrice) -> order_append(neworder);
    }
}

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

void book::cancelStopOrder(int orderid){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    order* Order = order_map.at(orderid);
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            Order -> get_parent_limit() -> deleteLimit(Order -> get_parent_limit());
        }
        deleteFromOrderMap(Order);
        delete Order;
    }
}

void book::modifyStopOrder(int orderid , bool buyorsell , int shares , int stopPrice){
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

void book::modifylimitorder(int idnumber,int newlimit,int newshares){
    auto& order = order_map.at(idnumber);
    if (order != nullptr){
        order -> cancel();
        if (order -> get_parent_limit() -> get_size() == 0){
            order -> get_parent_limit() -> deleteLimit(order -> get_parent_limit());
        }
        auto& limit_map = order -> get_buyorsell() ? limitbuy_map : limitsell_map;
        order -> modifyorder(newshares , newlimit);
        if (limit_map.find(newlimit) == limit_map.end()){
            addLimit(newlimit , order -> get_buyorsell());
        }
        limit_map.at(newlimit) -> order_append(order);
    }
};

int book::getLeftSideHeight(limit* Limit){
    limit* ptr = Limit -> get_leftchild();
    int count = 1;
    while (ptr != nullptr){
        ptr = ptr -> get_leftchild();
        count++;
    }
    return count;
}

int book::getRightSideHeight(limit* Limit){
    limit* ptr = Limit->get_rightchild();
    int count = 1;
    while (ptr != nullptr){
        ptr = ptr -> get_rightchild();
        count++;
    };
    return count;
};

limit* book::insert(limit* root , limit* newlimit , limit* parent){
    if (root == nullptr){
        newlimit -> setParent(parent);
        return newlimit;
    }
    if (newlimit -> get_limitPrice() < root -> get_limitPrice()){
        root -> setleftchild(insert(root->get_leftchild() , newlimit , root));
        root = balanceTree(root);
    }
    else if (newlimit -> get_limitPrice() > root -> get_limitPrice()){
        root -> setrightchild(insert(root->get_rightchild() , newlimit , root));
        root = balanceTree(root);
    }
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
    limit* root = insert(tree , newlimit);
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
void book::addLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice)
{
    auto AVLTreeBalanceCount = 0;
    // Account for order being executed immediately
    shares = LimitOrderAsMarketOrder(orderId, buyOrSell, shares, limitPrice);
    
    if (shares != 0)
    {
        order* newOrder = new order(orderId, buyOrSell, shares, limitPrice);
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

void book::addStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice)
{
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    // Account for stop limit order being executed immediately
    shares = stopLimitOrderAsLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
    
    if (shares != 0)
    {
        order* newOrder = new order(orderId, buyOrSell, shares, limitPrice);
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
    cancelStopOrder(orderId);
    addLimitOrder(orderId, buyOrSell, shares, limitPrice);
};

void book::cancelLimitOrder(int orderId)
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
        delete Order;
    }
}

int book::stopLimitOrderAsLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice)
{
    if (buyOrSell && lowestsell != nullptr && stopPrice <= lowestsell->get_limitPrice())
    {
        addLimitOrder(orderId, true, shares, limitPrice);
        return 0;
    } else if (!buyOrSell && highestbuy != nullptr && stopPrice >= highestbuy->get_limitPrice())
    {
        addLimitOrder(orderId, false, shares, limitPrice);
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
        delete headorder;
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
            LL_rebalance(Limit);
        }else{
            LR_rebalance(Limit);
        }
    }else if (b_factor < -1){
        if (get_b(Limit -> get_rightchild()) <= 0){
            RR_rebalance(Limit);
        }else{
            RL_rebalance(Limit);
        }
    }
    return Limit;
}

int book::get_b(limit* Limit){
    int left = getLeftSideHeight(Limit);
    int right = getRightSideHeight(Limit);
    int b_factor = std::max(left,right) + 1;
    return b_factor;
};

limit* book::LL_rebalance(limit* Limit){
    limit* newparent = Limit -> get_leftchild();
    Limit -> setleftchild(newparent -> get_rightchild());
    if (newparent -> get_rightchild() != nullptr){
        newparent -> get_rightchild() -> setParent(Limit);
    }
    newparent -> setrightchild(Limit);
    if (Limit -> get_parent() != nullptr){
        newparent -> setParent(Limit -> get_parent());
    } else {
        newparent -> setParent(nullptr);
        auto& tree = Limit -> getbuyorsell() ? buytree : selltree;
        tree = newparent;
    }
    Limit -> setParent(newparent);
    return newparent;
};//fixed rebalance
limit* book::RR_rebalance(limit* Limit){
    limit* newparent = Limit -> get_rightchild();
    Limit -> setrightchild(newparent -> get_leftchild());
    if (newparent -> get_leftchild() != nullptr){
        newparent -> get_leftchild() -> setParent(Limit);
    }
    newparent -> setleftchild(Limit);
    if (Limit -> get_parent() != nullptr){
        newparent -> setParent(Limit -> get_parent());
    } else {
        newparent -> setParent(nullptr);
        auto& tree = Limit -> getbuyorsell() ? buytree : selltree;
        tree = newparent;
    }
    Limit -> setParent(newparent);
    return newparent;
}//fixed rebalance

limit* book::LR_rebalance(limit* Limit){
    limit* newparent = Limit -> get_leftchild() -> get_rightchild();
    limit* temp = Limit -> get_leftchild();
    Limit -> setleftchild(newparent);
    newparent -> setleftchild(temp);
    return LL_rebalance(Limit);
}

limit* book::RL_rebalance(limit* Limit){
    limit* newparent = Limit -> get_rightchild() -> get_leftchild();
    limit* temp = Limit -> get_rightchild();
    Limit -> setleftchild(newparent);
    newparent -> setleftchild(temp);
    return RR_rebalance(Limit);
}

