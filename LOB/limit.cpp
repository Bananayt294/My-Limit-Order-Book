#include "order.hpp"
#include "limit.hpp"

limit::limit(int _limit_price, int _size, bool _buyorsell, int _totalshares)
    : tailOrder{nullptr}, headOrder{nullptr}, size{_size}, limit_price{_limit_price}, totalshares{_totalshares},
      leftchild{nullptr}, rightchild{nullptr}, parent{nullptr}, buyorsell{_buyorsell} {}

void limit::deleteLimit(limit* Limit){
    delete Limit;
}

order* limit::get_headOrder(){
    return headOrder;
}

order* limit::get_tailOrder(){
    return tailOrder;
}

limit* limit::get_leftchild(){
    return leftchild;
}
limit* limit::get_rightchild(){
    return rightchild;
}
limit* limit::get_parent(){
    return parent;
}

int limit::get_size(){
    return size;
}
int limit::get_limitPrice(){
    return limit_price;
}
int limit::get_totalshares(){
    return totalshares; 
}

void limit::setrightchild(limit* rightchild){
    this -> rightchild = rightchild;
}

void limit::setleftchild(limit* leftchild){
    this -> leftchild = leftchild;
}

void limit::setParent(limit* parent){
    this -> parent = parent;
}

bool limit::getbuyorsell(){
    return buyorsell;
}

void limit::order_append(order* order){
    if (headOrder == nullptr){
        headOrder = tailOrder = order;
    }else{
        tailOrder -> nextOrder = order;
        order -> prevOrder = tailOrder;
        order -> nextOrder = nullptr;
        tailOrder = order;
    };
    size +=1;
    totalshares += order->getshares();
    order-> parent_limit = this;
}

void limit::partiallyFillTotalVolume(int orderedShares){
    totalshares -= orderedShares;
}