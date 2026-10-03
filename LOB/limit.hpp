#ifndef LIMIT_HPP
#define LIMIT_HPP

// Forward declaration.
// We only use order* inside this class, so the full definition
// of order is not needed here.
class order;

class limit
{
    // order::cancel() and order::execute() directly access
    // private members of limit.
    friend class order;

private:
    order* tailOrder;
    order* headOrder;

    int size;
    int limit_price;
    int totalshares;

    limit* leftchild;
    limit* rightchild;
    limit* parent;

    bool buyorsell;

    // Cached AVL height
    int height;

public:
    limit(
        int _limit_price,
        int _size,
        bool _buyorsell,
        int _totalshares
    );

    // Orders
    order* get_headOrder();
    order* get_tailOrder();

    // Tree
    limit* get_leftchild();
    limit* get_rightchild();
    limit* get_parent();

    // Data
    int get_size();
    int get_limitPrice();
    int get_totalshares();
    bool getbuyorsell();

    // AVL height
    int getHeight();
    void setHeight(int h);

    // Tree setters
    void setrightchild(limit* rightchild);
    void setleftchild(limit* leftchild);
    void setParent(limit* parent);

    // Orders
    void order_append(order* order);
    void partiallyFillTotalVolume(int orderedShares);
};

#endif