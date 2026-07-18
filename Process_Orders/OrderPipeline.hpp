#ifndef ORDERPIPELINE_HPP
#define ORDERPIPELINE_HPP

#include <string>
#include <unordered_map>
#include <string_view>
#include <sstream>

class book;

class OrderPipeline {
private:
    book* m_book;

    using OrderFunction = void(OrderPipeline::*)(std::istringstream&);
    std::unordered_map<std::string_view, OrderFunction> orderFunctions;

    void processMarketOrder(std::istringstream& iss);
    void processAddLimitOrder(std::istringstream& iss);
    void processCancelLimitOrder(std::istringstream& iss);
    void processModifyLimitOrder(std::istringstream& iss);
    void processAddStopOrder(std::istringstream& iss);
    void processCancelStopOrder(std::istringstream& iss);
    void processModifyStopOrder(std::istringstream& iss);
    void processAddStopLimitOrder(std::istringstream& iss);
    void processCancelStopLimitOrder(std::istringstream& iss);
    void processModifyStopLimitOrder(std::istringstream& iss);

public:
    OrderPipeline(book* b);
    void processOrdersFromFile(const std::string& filename);
};

#endif