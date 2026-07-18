class limit;

class order{
    private:
        int Limit;
        order* prevOrder;
        order* nextOrder;
        int idnumber;
        bool buyorsell;
        int size;
        int shares;
        limit* parent_limit;

        friend class limit;
    public:
        void partiallyFillOrder(int OrderedShares);
        void setShares(int shares);
        void setSize(int size);
        order(int _idnumber , bool buyorsell , int shares, int limit);
        ~order();
        int get_Limit();
        int getshares();
        bool get_buyorsell();
        int get_size();
        int get_idNumber();
        limit* get_parent_limit();
        void cancel();
        void execute();
        void modifyorder(int newshares,int newlimit);
};