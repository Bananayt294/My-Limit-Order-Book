

class order;

class limit{
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
        
        friend class order;
    public:
        void deleteLimit(limit* Limit);
        limit(double _limit_price,double _size = 0 , bool _buyorsell,double _totalshares = 0);
        void setrightchild(limit* rightchild);
        void setleftchild(limit* leftchild);
        bool getbuyorsell();
        order* get_headOrder();
        order* get_tailOrder();
        limit* get_parent();
        limit* get_rightchild();
        limit* get_leftchild();
        int get_totalshares();
        int get_limitPrice();
        int get_size();
        void order_append(order* order);
        void partiallyFillTotalVolume(int orderedShares);
};