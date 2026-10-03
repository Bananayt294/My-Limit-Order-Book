#include "../Process_Orders/OrderPipeline.hpp"
#include "../LOB/book.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <chrono>

OrderPipeline::OrderPipeline(book* b) : m_book(b) {
  orderFunctions = {
    {"Market", &OrderPipeline::processMarketOrder},
    {"AddLimit", &OrderPipeline::processAddLimitOrder},
    {"AddMarketLimit", &OrderPipeline::processAddLimitOrder},
    {"CancelLimit", &OrderPipeline::processCancelLimitOrder},
    {"ModifyLimit", &OrderPipeline::processModifyLimitOrder},

    {"ReduceOrder", &OrderPipeline::processReduceOrder},
    {"ExecuteOrder", &OrderPipeline::processExecuteOrder},

    {"AddStop", &OrderPipeline::processAddStopOrder},
    {"CancelStop", &OrderPipeline::processCancelStopOrder},
    {"ModifyStop", &OrderPipeline::processModifyStopOrder},
    {"AddStopLimit", &OrderPipeline::processAddStopLimitOrder},
    {"CancelStopLimit", &OrderPipeline::processCancelStopLimitOrder},
    {"ModifyStopLimit", &OrderPipeline::processModifyStopLimitOrder}
};
}

void OrderPipeline::processOrdersFromFile(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: "
                  << filename << std::endl;
        return;
    }

    std::string line;
    long long processed = 0;

    auto lastTime = std::chrono::steady_clock::now();

    while (std::getline(file, line)) {

        std::istringstream iss(line);

        std::string orderType;
        iss >> orderType;

        auto it = orderFunctions.find(orderType);

        if (it != orderFunctions.end()) {

            (this->*(it->second))(iss);

            processed++;

            if (processed % 1000 == 0) {

                auto now = std::chrono::steady_clock::now();

                auto elapsed =
                    std::chrono::duration_cast<
                        std::chrono::milliseconds
                    >(now - lastTime);

                std::cout
                    << "Processed: "
                    << processed
                    << " | Last 1000: "
                    << elapsed.count()
                    << " ms"
                    << std::endl;

                lastTime = now;
            }
        }
    }

    std::cout
        << "Finished processing: "
        << processed
        << " orders."
        << std::endl;
}
void OrderPipeline::processMarketOrder(std::istringstream& iss) {
    int orderId, shares;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares;
    m_book->MarketOrderHelper(orderId, buyOrSell, shares);
}

void OrderPipeline::processAddLimitOrder(std::istringstream& iss) {
    int orderId, shares, limitPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares >> limitPrice;
    m_book->AddLimitOrder(orderId, buyOrSell, shares, limitPrice);
}

void OrderPipeline::processCancelLimitOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->CancelLimitOrder(orderId);
}

void OrderPipeline::processModifyLimitOrder(std::istringstream& iss) {
    int orderId, newShares, newLimit;
    iss >> orderId >> newShares >> newLimit;
    m_book->ModifyLimitOrder(orderId, newShares, newLimit);
}

void OrderPipeline::processAddStopOrder(std::istringstream& iss) {
    int orderId, shares, stopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares >> stopPrice;
    m_book->AddStopOrder(orderId, buyOrSell, shares, stopPrice);
}

void OrderPipeline::processCancelStopOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->CancelStopOrder(orderId);
}

void OrderPipeline::processModifyStopOrder(std::istringstream& iss) {
    int orderId, newShares, newStopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> newShares >> newStopPrice;
    m_book->ModifyStopOrder(orderId , newShares, newStopPrice);
}

void OrderPipeline::processAddStopLimitOrder(std::istringstream& iss) {
    int orderId, shares, limitPrice, stopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares >> stopPrice >> limitPrice;
    m_book->AddStopLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
}

void OrderPipeline::processCancelStopLimitOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->CancelStopLimitOrder(orderId);
}

void OrderPipeline::processModifyStopLimitOrder(std::istringstream& iss) {
    int orderId, newShares, newStopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> newShares >> newStopPrice;
    m_book->ModifyStopLimitOrder(orderId, buyOrSell, newShares, newStopPrice);
}

void OrderPipeline::processReduceOrder(std::istringstream& iss)
{
    int orderId, shares;

    iss >> orderId >> shares;

    m_book->ReduceOrder(orderId, shares);
}

void OrderPipeline::processExecuteOrder(std::istringstream& iss)
{
    int orderId, shares;

    iss >> orderId >> shares;

    m_book->ExecuteOrder(orderId, shares);
}