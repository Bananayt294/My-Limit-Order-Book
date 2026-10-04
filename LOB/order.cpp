#include "limit.hpp"
#include "order.hpp"

order::order(int _idnumber , bool buyorsell , int shares, int limit)
    : idnumber{_idnumber}, buyorsell{buyorsell}, shares{shares}, Limit{limit}, parent_limit{nullptr}, prevOrder{nullptr}, nextOrder{nullptr} {}

order::~order() = default;

void order::setShares(int shares){
    this -> shares = shares;
}

void order::setSize(int size){
    this -> size = size;
}

int order::getshares(){
    return shares;
}

int order::get_Limit(){
    return Limit;
}

bool order::get_buyorsell(){
    return buyorsell;
}

int order::get_size(){
    return size;
}

int order::get_idNumber(){
    return idnumber;
}

limit* order::get_parent_limit(){
    return parent_limit;
}

void order::cancel(){
  if (parent_limit == nullptr) {
    prevOrder = nullptr;
    nextOrder = nullptr;
    return;
  }

  if (prevOrder == nullptr){
     parent_limit -> headOrder = nextOrder;
  }else {
    prevOrder -> nextOrder = nextOrder;
  }
  if (nextOrder == nullptr){
    parent_limit -> tailOrder = prevOrder;
  }else {
    nextOrder -> prevOrder = prevOrder;
  }
  parent_limit -> totalshares -= shares;
  parent_limit -> size -= 1;
  prevOrder = nullptr;
  nextOrder = nullptr;
}

void order::execute(){
    if (parent_limit == nullptr) {
        shares = 0;
        prevOrder = nullptr;
        nextOrder = nullptr;
        return;
    }

    if (prevOrder){
        prevOrder->nextOrder = nextOrder;
    }
    else{
        parent_limit->headOrder = nextOrder;
    }
    if (nextOrder){
        nextOrder->prevOrder = prevOrder;
    }
    else{
        parent_limit->tailOrder = prevOrder;
    }
    parent_limit->totalshares -= shares;
    parent_limit->size--;
    shares = 0;

    prevOrder = nullptr;
    nextOrder = nullptr;
}

void order::modifyorder(int newshares , int newlimit){
  this -> shares = newshares;
  this -> Limit = newlimit;
  prevOrder = nullptr;
  nextOrder = nullptr;
  parent_limit = nullptr;
}

void order::partiallyFillOrder(int Orderedshares){
    this -> shares -= Orderedshares;
    if (parent_limit != nullptr) {
        parent_limit -> partiallyFillTotalVolume(Orderedshares);
    }
}
