#include "limit.hpp"
#include "book.hpp"
#include "order.hpp"
#include "order_pool.hpp"
#include "limit_pool.hpp"
#include "iostream"
#include <algorithm>
#include <random>
#include <iterator>
#include <vector>

book::book()
    : buytree{nullptr}, selltree{nullptr}, highestbuy{nullptr}, lowestsell{nullptr}, stopbuytree{nullptr}, stopselltree{nullptr}, order_allocator{new order_pool()}, limit_allocator{new limit_pool()} {}

book::~book() {
    for (auto& [id, order] : order_map) {
        order_allocator->release(order);
    }
    order_map.clear();

    for (auto& [limitPrice, limit] : limitbuy_map) {
        limit_allocator->release(limit);
    }
    limitbuy_map.clear();

    for (auto& [limitPrice, limit] : limitsell_map) {
        limit_allocator->release(limit);
    }
    limitsell_map.clear();

    for (auto& [stopPrice, stopLevel] : stopmap) {
        limit_allocator->release(stopLevel);
    }
    stopmap.clear();

    delete order_allocator;
    delete limit_allocator;
}

void book::AddStopOrder(int orderid ,bool buyorsell , int shares , int stopPrice){
    int executedOrdersCount = 0;
    int AVLTreeBalanceCount = 0;

    shares = stopOrderAsMarketOrder(orderid , buyorsell , shares , stopPrice);

    if (shares != 0){
        order* neworder = order_allocator->allocate(orderid , buyorsell , shares , 0);
        if (neworder == nullptr) {
            std::cerr << "Order pool exhausted while adding stop order " << orderid << std::endl;
            return;
        }
        order_map.emplace(orderid , neworder);
        if (stopmap.find(stopPrice) == stopmap.end()){
            limit* newlimit = limit_allocator->allocate(stopPrice, 0, buyorsell, 0);
            if (newlimit == nullptr) {
                std::cerr << "Limit pool exhausted while adding stop level " << stopPrice << std::endl;
                order_allocator->release(neworder);
                deleteFromOrderMap(neworder);
                return;
            }
            stopmap.emplace(stopPrice,newlimit);
        }
        stopmap.at(stopPrice) -> order_append(neworder);
    }
}

void book::CancelStopLimitOrder(int orderId){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    auto it = order_map.find(orderId);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    order* Order = it->second;
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            deleteLimit(Order -> get_parent_limit());
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
    auto it = order_map.find(orderid);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    order* Order = it->second;
    if (Order != nullptr){
        limit* parentLimit= Order->get_parent_limit();
        Order -> cancel();
        if (parentLimit -> get_size() == 0){
            deleteLimit(parentLimit);
        }
        deleteFromOrderMap(Order);
        order_allocator->release(Order);
    }
}

void book::ModifyStopLimitOrder(int orderId, int newShares, int newLimitPrice, int newStopPrice){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    auto it = order_map.find(orderId);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    order* Order = it->second;
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            deleteLimit(Order -> get_parent_limit());
        }
        Order -> modifyorder(newShares , newLimitPrice);
        if (stopmap.find(newStopPrice) == stopmap.end()){
            addLimit(newStopPrice , Order -> get_buyorsell());
        }
        stopmap.at(newStopPrice) -> order_append(Order);
    }
}

void book::ModifyStopOrder(int orderid , int shares , int stopPrice){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    auto it = order_map.find(orderid);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    order* Order = it->second;
    if (Order != nullptr){
        Order -> cancel();
        if (Order -> get_parent_limit() -> get_size() == 0){
            deleteLimit(Order -> get_parent_limit());
        }
        Order -> modifyorder(shares , 0);
        if (stopmap.find(stopPrice) == stopmap.end()){
            addLimit(stopPrice , Order -> get_buyorsell());
        }
        stopmap.at(stopPrice) -> order_append(Order);
    }
}

void book::ModifyLimitOrder(int orderId, int newShares, int newLimit){
    auto it = order_map.find(orderId);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    auto& order = it->second;
    if (order != nullptr){
        order -> cancel();
        if (order -> get_parent_limit() -> get_size() == 0){
            deleteLimit(order -> get_parent_limit());
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

void book::updateHeight(limit* Limit)
{
    if (Limit == nullptr)
        return;

    int leftHeight = 0;
    int rightHeight = 0;

    if (Limit->get_leftchild() != nullptr)
        leftHeight = Limit->get_leftchild()->getHeight();

    if (Limit->get_rightchild() != nullptr)
        rightHeight = Limit->get_rightchild()->getHeight();

    Limit->setHeight(
        1 + std::max(leftHeight, rightHeight)
    );
}

int book::getLimitHeight(limit* node)
{
    if (node == nullptr)
        return 0;

    return node->getHeight();
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

limit* book::insert(
    limit* root,
    limit* newlimit,
    limit* parent
)
{
    if (root == nullptr) {
        newlimit->setParent(parent);
        newlimit->setHeight(1);
        return newlimit;
    }

    if (newlimit->get_limitPrice() < root->get_limitPrice()) {

        root->setleftchild(
            insert(
                root->get_leftchild(),
                newlimit,
                root
            )
        );

    }
    else if (newlimit->get_limitPrice() > root->get_limitPrice()) {

        root->setrightchild(
            insert(
                root->get_rightchild(),
                newlimit,
                root
            )
        );
    }

    return balanceTree(root);
}

void book::addLimit(int limit_price , bool buyorsell){
    auto& Limitmap = buyorsell ? limitbuy_map : limitsell_map;
    auto& tree = buyorsell ? buytree : selltree;
    auto& bookedge = buyorsell ? highestbuy : lowestsell;

   limit* newlimit = limit_allocator->allocate(limit_price , 0 ,buyorsell , 0);
   Limitmap.emplace(limit_price , newlimit);
   if (tree == nullptr){
    tree = newlimit;
    bookedge = newlimit;
   }else{
    tree = insert(tree , newlimit);
    updateBookEdgeInsert(newlimit);
   }
}

void book::updateBookEdgeInsert(limit* newlimit){
    auto& tree = newlimit -> getbuyorsell() ? buytree : selltree;
    if(tree == buytree){
        if (highestbuy == nullptr){
            highestbuy = newlimit;
            return;
        }
        if(newlimit -> get_limitPrice() > highestbuy -> get_limitPrice()){
            highestbuy = newlimit;
        }
    }else{ 
        if (lowestsell == nullptr){
            lowestsell = newlimit;
            return;
        }
        if (tree == selltree){
            if(newlimit -> get_limitPrice() < lowestsell -> get_limitPrice()){
                lowestsell = newlimit;
            };
        };
    };
};
//MUST UPDATE ADD LIMIT ORDER FUNCTION
void book::AddLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice){
    auto AVLTreeBalanceCount = 0;
    // Account for order being executed immediately
    shares = LimitOrderAsMarketOrder(orderId, buyOrSell, shares, limitPrice);
    
    if (shares != 0)
    {
        order* newOrder = order_allocator->allocate(orderId, buyOrSell, shares, limitPrice);
        if (newOrder == nullptr) {
            std::cerr << "Order pool exhausted while adding limit order " << orderId << std::endl;
            return;
        }
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


void book::ReduceOrder(int orderId, int shares)
{
    auto it = order_map.find(orderId);

    if (it == order_map.end() || it->second == nullptr) {
        return;
    }

    order* Order = it->second;

    if (shares <= 0) {
        return;
    }

    if (shares >= Order->getshares()) {
        CancelLimitOrder(orderId);
        return;
    }

    Order->partiallyFillOrder(shares);
}

void book::ExecuteOrder(int orderId, int shares)
{
    auto it = order_map.find(orderId);

    if (it == order_map.end() || it->second == nullptr) {
        return;
    }

    order* Order = it->second;

    if (shares <= 0) {
        return;
    }

    if (shares < Order->getshares()) {
        Order->partiallyFillOrder(shares);
        return;
    }

    limit* parentLimit = Order->get_parent_limit();

    Order->execute();

    if (parentLimit != nullptr && parentLimit->get_size() == 0) {
        deleteLimit(parentLimit);
    }

    deleteFromOrderMap(Order);
    order_allocator->release(Order);
}


void book::deleteLimit(limit* Limit)
{
    if (Limit == nullptr) {
        return;
    }

    auto buyIt =
        limitbuy_map.find(
            Limit->get_limitPrice()
        );

    auto sellIt =
        limitsell_map.find(
            Limit->get_limitPrice()
        );

    bool foundInBuyMap =
        (
            buyIt != limitbuy_map.end() &&
            buyIt->second == Limit
        );

    bool foundInSellMap =
        (
            sellIt != limitsell_map.end() &&
            sellIt->second == Limit
        );

    // ---------------------------------------------
    // Stop order level
    // ---------------------------------------------
    if (!foundInBuyMap && !foundInSellMap) {

        auto stopIt =
            stopmap.find(
                Limit->get_limitPrice()
            );

        if (
            stopIt != stopmap.end() &&
            stopIt->second == Limit
        ) {
            stopmap.erase(stopIt);
            limit_allocator->release(Limit);
        }

        return;
    }

    // ---------------------------------------------
    // Buy side
    // ---------------------------------------------
    if (foundInBuyMap) {

        int price = Limit->get_limitPrice();

        limitbuy_map.erase(price);

        buytree =
            deleteNode(
                buytree,
                price
            );

        // Find new highest buy
        highestbuy = buytree;

        while (
            highestbuy != nullptr &&
            highestbuy->get_rightchild() != nullptr
        ) {
            highestbuy =
                highestbuy->get_rightchild();
        }

        return;
    }

    // ---------------------------------------------
    // Sell side
    // ---------------------------------------------

    int price = Limit->get_limitPrice();

    limitsell_map.erase(price);

    selltree =
        deleteNode(
            selltree,
            price
        );

    // Find new lowest sell
    lowestsell = selltree;

    while (
        lowestsell != nullptr &&
        lowestsell->get_leftchild() != nullptr
    ) {
        lowestsell =
            lowestsell->get_leftchild();
    }
}


// deleteNode removes the limit node with the specified price from the AVL tree and returns the updated subtree root.
limit* book::deleteNode(limit* root, int limitprice)
{
    if (root == nullptr) {
        return nullptr;
    }

    // Search left
    if (limitprice < root->get_limitPrice()) {

        limit* newLeft =
            deleteNode(
                root->get_leftchild(),
                limitprice
            );

        root->setleftchild(newLeft);

        if (newLeft != nullptr) {
            newLeft->setParent(root);
        }

        return balanceTree(root);
    }

    // Search right
    if (limitprice > root->get_limitPrice()) {

        limit* newRight =
            deleteNode(
                root->get_rightchild(),
                limitprice
            );

        root->setrightchild(newRight);

        if (newRight != nullptr) {
            newRight->setParent(root);
        }

        return balanceTree(root);
    }

    // ------------------------------------------------
    // We found the node
    // ------------------------------------------------

    limit* parent = root->get_parent();

    // Case 1: no children
    if (
        root->get_leftchild() == nullptr &&
        root->get_rightchild() == nullptr
    ) {

        root->setParent(nullptr);

        limit_allocator->release(root);

        return nullptr;
    }

    // Case 2: only right child
    if (root->get_leftchild() == nullptr) {

        limit* child = root->get_rightchild();

        child->setParent(parent);

        root->setrightchild(nullptr);
        root->setParent(nullptr);

        limit_allocator->release(root);

        return child;
    }

    // Case 3: only left child
    if (root->get_rightchild() == nullptr) {

        limit* child = root->get_leftchild();

        child->setParent(parent);

        root->setleftchild(nullptr);
        root->setParent(nullptr);

        limit_allocator->release(root);

        return child;
    }

    // ------------------------------------------------
    // Case 4: two children
    // ------------------------------------------------

    limit* successor = nullptr;

    limit* newRight =
        removeMinimum(
            root->get_rightchild(),
            successor
        );

    limit* leftSubtree = root->get_leftchild();

    // Successor receives left subtree
    successor->setleftchild(leftSubtree);

    if (leftSubtree != nullptr) {
        leftSubtree->setParent(successor);
    }

    // Successor receives remaining right subtree
    successor->setrightchild(newRight);

    if (newRight != nullptr) {
        newRight->setParent(successor);
    }

    // Connect successor to old parent
    successor->setParent(parent);

    // Remove old node
    root->setleftchild(nullptr);
    root->setrightchild(nullptr);
    root->setParent(nullptr);

    limit_allocator->release(root);

    return balanceTree(successor);
}

limit* book::removeMinimum(
    limit* node,
    limit*& minimum
)
{
    if (node->get_leftchild() == nullptr) {

        minimum = node;

        limit* rightChild =
            node->get_rightchild();

        if (rightChild != nullptr) {
            rightChild->setParent(node->get_parent());
        }

        node->setrightchild(nullptr);
        node->setParent(nullptr);

        return rightChild;
    }

    limit* newLeft =
        removeMinimum(
            node->get_leftchild(),
            minimum
        );

    node->setleftchild(newLeft);

    if (newLeft != nullptr) {
        newLeft->setParent(node);
    }

    return balanceTree(node);
}

void book::AddStopLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    // Account for stop limit order being executed immediately
    shares = StopLimitOrderAsLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
    
    if (shares != 0){
        order* newOrder = order_allocator->allocate(orderId, buyOrSell, shares, limitPrice);
        if (newOrder == nullptr) {
            std::cerr << "Order pool exhausted while adding stop-limit order " << orderId << std::endl;
            return;
        }
        order_map.emplace(orderId, newOrder);

        if (stopmap.find(stopPrice) == stopmap.end()){
            limit* newStopLimit =  limit_allocator->allocate(stopPrice , 0 , buyOrSell , 0);
            if (newStopLimit == nullptr) {
                std::cerr << "Limit pool exhausted while adding stop-limit level " << stopPrice << std::endl;
                order_allocator->release(newOrder);
                deleteFromOrderMap(newOrder);
                return;
            }
            stopmap.emplace(stopPrice , newStopLimit);
        }
        stopmap.at(stopPrice)->order_append(newOrder);
        // stopLimitOrders.insert(newOrder);
    }
}

void book::executeStopOrders(bool buyOrSell){
    auto* bookEdge = buyOrSell ? lowestsell : highestbuy;
    while (bookEdge != nullptr){
        order* headOrder = bookEdge->get_headOrder();
        if (headOrder == nullptr) {
            auto* toDelete = bookEdge;
            bookEdge = buyOrSell ? lowestsell : highestbuy;
            deleteLimit(toDelete);
            continue;
        }
        if (headOrder->get_buyorsell() == buyOrSell){
            stopLimitOrderToLimitOrder(headOrder, buyOrSell);
            bookEdge = buyOrSell ? lowestsell : highestbuy;
        }
        else{
            break;
        }
    }
}

void book::stopLimitOrderToLimitOrder(order* stopLimitOrder, bool buyOrSell){
    int orderId = stopLimitOrder->get_idNumber();
    int shares = stopLimitOrder->getshares();
    int limitPrice = stopLimitOrder->get_Limit();
    CancelStopOrder(orderId);
    AddLimitOrder(orderId, buyOrSell, shares, limitPrice);
};

order* book::getRandomOrder(int key, std::mt19937 gen) const{
    if (key == 0)
    {
        if (limitOrders.size() > 10000)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, limitOrders.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = limitOrders.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    } else if (key == 1)
    {
        if (stopOrders.size() > 500)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, stopOrders.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = stopOrders.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    } else if (key == 2)
    {
        if (stopLimitOrders.size() > 500)
        {
            // Generate a random index within the range of the hash set size
            std::uniform_int_distribution<> mapDist(0, stopLimitOrders.size() - 1);
            int randomIndex = mapDist(gen);

            // Access the element at the random index directly
            auto it = stopLimitOrders.begin();
            std::advance(it, randomIndex);
            return *it;
        }
        return nullptr;
    }
    return nullptr;
}

void book::CancelLimitOrder(int orderId){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    auto it = order_map.find(orderId);
    if (it == order_map.end() || it->second == nullptr) {
        return;
    }
    order* Order = it->second;
    if (Order != nullptr){
        Order->cancel();
        if (Order->get_parent_limit()->get_size() == 0){
            deleteLimit(Order->get_parent_limit());
        }
        deleteFromOrderMap(Order);
        order_allocator->release(Order);
    }
}

int book::StopLimitOrderAsLimitOrder(int orderId, bool buyOrSell, int shares, int limitPrice, int stopPrice){
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

int book::LimitOrderAsMarketOrder(int orderId, bool buyOrSell, int shares, int limitPrice){
    if (buyOrSell)
    {
        while (lowestsell != nullptr && shares != 0 && lowestsell->get_limitPrice() <= limitPrice)
        {
            if (shares <= lowestsell->get_totalshares()){
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
            if (shares <= highestbuy->get_totalshares()){
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
    auto* bookedge = buyorsell ? lowestsell : highestbuy;
    auto executedOrdersCount = 0;

    while (bookedge != nullptr) {
        order* headorder = bookedge->get_headOrder();
        if (headorder == nullptr) {
            auto* toDelete = bookedge;
            bookedge = buyorsell ? lowestsell : highestbuy;
            deleteLimit(toDelete);
            continue;
        }

        if (headorder->getshares() > shares) {
            headorder->partiallyFillOrder(shares);
            executedOrdersCount++;
            break;
        }

        shares -= headorder->getshares();
        headorder->execute();

        auto* currentLimit = bookedge;
        if (currentLimit->get_size() == 0) {
            bookedge = buyorsell ? lowestsell : highestbuy;
            deleteLimit(currentLimit);
        } else {
            bookedge = buyorsell ? lowestsell : highestbuy;
        }

        deleteFromOrderMap(headorder);
        order_allocator->release(headorder);
        executedOrdersCount++;
    }
}

void book::marketOrder(int orderid, bool buyorsell, int shares){
    auto executedOrdersCount = 0;
    auto AVLTreeBalanceCount = 0;
    MarketOrderHelper(orderid, buyorsell, shares);

    executeStopOrders(buyorsell);
}

void book::deleteFromOrderMap(order* Order){
    auto it = order_map.find(Order -> get_idNumber());
    if (it != order_map.end()){
        order_map.erase(it);
    }
};

std::vector<int> book::PreorderTraversal(limit* root){
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

std::vector<int> book::PostorderTraversal(limit* root){
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

std::vector<int> book::InorderTraversal(limit* root){
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

limit* book::balanceTree(limit* Limit)
{
    if (Limit == nullptr)
        return nullptr;

    updateHeight(Limit);

    int b_factor = get_b(Limit);

    if (b_factor > 1) {

        if (get_b(Limit->get_leftchild()) >= 0) {
            return LL_rebalance(Limit);
        }
        else {
            return LR_rebalance(Limit);
        }
    }

    if (b_factor < -1) {

        if (get_b(Limit->get_rightchild()) <= 0) {
            return RR_rebalance(Limit);
        }
        else {
            return RL_rebalance(Limit);
        }
    }

    return Limit;
}

int book::get_b(limit* Limit)
{
    if (Limit == nullptr)
        return 0;

    int leftHeight = 0;
    int rightHeight = 0;

    if (Limit->get_leftchild() != nullptr)
        leftHeight = Limit->get_leftchild()->getHeight();

    if (Limit->get_rightchild() != nullptr)
        rightHeight = Limit->get_rightchild()->getHeight();

    return leftHeight - rightHeight;
}

limit* book::LL_rebalance(limit* Limit)
{
    limit* newparent = Limit->get_leftchild();
    limit* parent = Limit->get_parent();

    if (parent != nullptr) {

        newparent->setParent(parent);

        if (parent->get_leftchild() == Limit) {
            parent->setleftchild(newparent);
        }
        else if (parent->get_rightchild() == Limit) {
            parent->setrightchild(newparent);
        }
    }
    else {

        newparent->setParent(nullptr);

        auto& tree =
            Limit->getbuyorsell()
                ? buytree
                : selltree;

        tree = newparent;
    }

    limit* middle = newparent->get_rightchild();

    Limit->setleftchild(middle);

    if (middle != nullptr) {
        middle->setParent(Limit);
    }

    newparent->setrightchild(Limit);
    Limit->setParent(newparent);

    // Update lower node first
    updateHeight(Limit);

    // Then update new root
    updateHeight(newparent);

    return newparent;
}

limit* book::RR_rebalance(limit* Limit)
{
    limit* newparent = Limit->get_rightchild();
    limit* parent = Limit->get_parent();

    if (parent != nullptr) {

        newparent->setParent(parent);

        if (parent->get_leftchild() == Limit) {
            parent->setleftchild(newparent);
        }
        else if (parent->get_rightchild() == Limit) {
            parent->setrightchild(newparent);
        }
    }
    else {

        newparent->setParent(nullptr);

        auto& tree =
            Limit->getbuyorsell()
                ? buytree
                : selltree;

        tree = newparent;
    }

    limit* middle = newparent->get_leftchild();

    Limit->setrightchild(middle);

    if (middle != nullptr) {
        middle->setParent(Limit);
    }

    newparent->setleftchild(Limit);
    Limit->setParent(newparent);

    // Update lower node first
    updateHeight(Limit);

    // Then update new root
    updateHeight(newparent);

    return newparent;
}

limit* book::LR_rebalance(limit* Limit)
{
    RR_rebalance(Limit->get_leftchild());
    return LL_rebalance(Limit);
}

limit* book::RL_rebalance(limit* Limit)
{
    LL_rebalance(Limit->get_rightchild());
    return RR_rebalance(Limit);
}