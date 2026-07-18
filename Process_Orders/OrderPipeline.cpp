#include "OrderPipeline.hpp"
#include "../book.hpp"
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
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::ofstream csvFile("order_processing_times.csv", std::ios::trunc); // Open in append mode
    if (!csvFile.is_open()) {
        std::cerr << "Error opening CSV file for writing." << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string orderType;
        iss >> orderType;

        auto it = orderFunctions.find(orderType);
            if (it != orderFunctions.end()) {
                auto start = std::chrono::steady_clock::now();

                (this->*(it->second))(iss);

                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                
                if (orderType == "AddLimit")
                {
                    csvFile << orderType << "," << duration.count() << std::endl;
                } else {
                    csvFile << orderType << "," << duration.count() << std::endl;
                }
                
            } else {
                std::cerr << "Unknown order type: " << orderType << std::endl;
            }
    }
    file.close();
    csvFile.close();
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
    m_book->addLimitOrder(orderId, buyOrSell, shares, limitPrice);
}

void OrderPipeline::processCancelLimitOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->cancelLimitOrder(orderId);
}

void OrderPipeline::processModifyLimitOrder(std::istringstream& iss) {
    int orderId, newShares, newLimit;
    iss >> orderId >> newShares >> newLimit;
    m_book->modifylimitorder(orderId, newLimit, newShares);
}

void OrderPipeline::processAddStopOrder(std::istringstream& iss) {
    int orderId, shares, stopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares >> stopPrice;
    m_book->addStopOrder(orderId, buyOrSell, shares, stopPrice);
}

void OrderPipeline::processCancelStopOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->cancelStopOrder(orderId);
}

void OrderPipeline::processModifyStopOrder(std::istringstream& iss) {
    int orderId, newShares, newStopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> newShares >> newStopPrice;
    m_book->modifyStopOrder(orderId, buyOrSell, newShares, newStopPrice);
}

void OrderPipeline::processAddStopLimitOrder(std::istringstream& iss) {
    int orderId, shares, limitPrice, stopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> shares >> stopPrice >> limitPrice;
    m_book->addStopLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
}

void OrderPipeline::processCancelStopLimitOrder(std::istringstream& iss) {
    int orderId;
    iss >> orderId;
    m_book->cancelStopOrder(orderId);
}

void OrderPipeline::processModifyStopLimitOrder(std::istringstream& iss) {
    int orderId, newShares, newStopPrice;
    bool buyOrSell;
    iss >> orderId >> buyOrSell >> newShares >> newStopPrice;
    m_book->modifyStopOrder(orderId, buyOrSell, newShares, newStopPrice);
}
