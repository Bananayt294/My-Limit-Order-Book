#include "GenerateOrders.hpp"
#include "../LOB/book.hpp"
#include "../LOB/order.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {
int safeHighestBuyPrice(Book* book)
{
    if (book == nullptr || book->getHighestBuy() == nullptr) {
        return 300;
    }
    return book->getHighestBuy()->get_limitPrice();
}

int safeLowestSellPrice(Book* book)
{
    if (book == nullptr || book->getLowestSell() == nullptr) {
        return 300;
    }
    return book->getLowestSell()->get_limitPrice();
}
} // namespace

GenerateOrders::GenerateOrders(Book* _book) : book(_book), gen(rd()) {}

void GenerateOrders::market()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::uniform_int_distribution<> buyOrSellDist(0, 1);

    const int shares = sharesDist(gen);
    const bool buyOrSell = buyOrSellDist(gen);

    file << "Market " << orderId << " " << buyOrSell << " " << shares << std::endl;
    book->marketOrder(orderId, buyOrSell, shares);
    ++orderId;
}

void GenerateOrders::addLimit()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> limitPriceDist(300, 50);
    std::uniform_int_distribution<> buyOrSellDist(0, 1);

    const int shares = sharesDist(gen);
    const bool buyOrSell = buyOrSellDist(gen);
    int limitPrice = 300;
    const int highestBuyPrice = safeHighestBuyPrice(book);
    const int lowestSellPrice = safeLowestSellPrice(book);

    if (buyOrSell)
    {
        do {
            limitPrice = limitPriceDist(gen);
        } while (limitPrice >= lowestSellPrice);
    } else {
        do {
            limitPrice = limitPriceDist(gen);
        } while (limitPrice <= highestBuyPrice);
    }

    file << "AddLimit " << orderId << " " << buyOrSell << " " << shares << " " << limitPrice << std::endl;
    book->addLimitOrder(orderId, buyOrSell, shares, limitPrice);
    ++orderId;
}

void GenerateOrders::cancelLimit()
{
    Order* order = book->getRandomOrder(0, gen);

    if (order == nullptr)
    {
        addLimit();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    file << "CancelLimit " << currentOrderId << std::endl;
    book->cancelLimitOrder(currentOrderId);
}

void GenerateOrders::modifyLimit()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> limitPriceDist(safeHighestBuyPrice(book), 50);

    int shares = sharesDist(gen);

    Order* order = book->getRandomOrder(0, gen);

    if (order == nullptr)
    {
        addLimit();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    const bool buyOrSell = order->get_buyorsell();
    int limitPrice = 300;
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        do {
            limitPrice = limitPriceDist(gen);
        } while (limitPrice >= lowestSellPrice);
    } else {
        do {
            limitPrice = limitPriceDist(gen);
        } while (limitPrice <= highestBuyPrice);
    }
    file << "ModifyLimit " << currentOrderId << " " << shares << " " << limitPrice << std::endl;
    book->modifyLimitOrder(currentOrderId, shares, limitPrice);
}

void GenerateOrders::addLimitMarket()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::uniform_int_distribution<> buyOrSellDist(0, 1);

    int shares = sharesDist(gen);
    int limitPrice = 300;
    const bool buyOrSell = buyOrSellDist(gen);
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        limitPrice = lowestSellPrice + 1;
    } else {
        limitPrice = highestBuyPrice - 1;
    }

    file << "AddMarketLimit " << orderId << " " << buyOrSell << " " << shares << " " << limitPrice << std::endl;
    book->addLimitOrder(orderId, buyOrSell, shares, limitPrice);
    ++orderId;
}

void GenerateOrders::addStop()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> stopPriceDist(safeHighestBuyPrice(book), 50);
    std::uniform_int_distribution<> buyOrSellDist(0, 1);

    const int shares = sharesDist(gen);
    const bool buyOrSell = buyOrSellDist(gen);

    int stopPrice = 300;
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice <= lowestSellPrice);
    } else {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice >= highestBuyPrice);
    }

    file << "AddStop " << orderId << " " << buyOrSell << " " << shares << " " << stopPrice << std::endl;
    book->addStopOrder(orderId, buyOrSell, shares, stopPrice);
    ++orderId;
}

void GenerateOrders::cancelStop()
{
    Order* order = book->getRandomOrder(1, gen);

    if (order == nullptr)
    {
        addStop();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    file << "CancelStop " << currentOrderId << std::endl;
    book->cancelStopOrder(currentOrderId);
}

void GenerateOrders::modifyStop()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> stopPriceDist(safeHighestBuyPrice(book), 50);

    int shares = sharesDist(gen);

    Order* order = book->getRandomOrder(1, gen);

    if (order == nullptr)
    {
        addStop();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    const bool buyOrSell = order->get_buyorsell();
    int stopPrice = 300;
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice <= lowestSellPrice);
    } else {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice >= highestBuyPrice);
    }
    file << "ModifyStop " << currentOrderId << " " << shares << " " << stopPrice << std::endl;
    book->modifyStopOrder(currentOrderId, buyOrSell, shares, stopPrice);
}

void GenerateOrders::addStopLimit()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> stopPriceDist(safeHighestBuyPrice(book), 50);
    std::uniform_int_distribution<> limitPriceDist(1, 5);
    std::uniform_int_distribution<> buyOrSellDist(0, 1);

    const int shares = sharesDist(gen);
    const bool buyOrSell = buyOrSellDist(gen);

    int stopPrice = 300;
    int limitPrice = 300;
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice <= lowestSellPrice);
        limitPrice = stopPrice + limitPriceDist(gen);
    } else {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice >= highestBuyPrice);
        limitPrice = stopPrice - limitPriceDist(gen);
    }

    file << "AddStopLimit " << orderId << " " << buyOrSell << " " << shares << " " << limitPrice << " " << stopPrice << std::endl;
    book->addStopLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
    ++orderId;
}

void GenerateOrders::cancelStopLimit()
{
    Order* order = book->getRandomOrder(2, gen);

    if (order == nullptr)
    {
        addStopLimit();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    file << "CancelStopLimit " << currentOrderId << std::endl;
    book->cancelStopLimitOrder(currentOrderId);
}

void GenerateOrders::modifyStopLimit()
{
    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> stopPriceDist(safeHighestBuyPrice(book), 50);
    std::uniform_int_distribution<> limitPriceDist(1, 5);

    int shares = sharesDist(gen);

    Order* order = book->getRandomOrder(2, gen);

    if (order == nullptr)
    {
        addStopLimit();
        return;
    }
    const int currentOrderId = order->get_idNumber();
    const bool buyOrSell = order->get_buyorsell();
    int stopPrice = 300;
    int limitPrice = 300;
    const int lowestSellPrice = safeLowestSellPrice(book);
    const int highestBuyPrice = safeHighestBuyPrice(book);

    if (buyOrSell)
    {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice <= lowestSellPrice);
        limitPrice = stopPrice + limitPriceDist(gen);
    } else {
        do {
            stopPrice = stopPriceDist(gen);
        } while (stopPrice >= highestBuyPrice);
        limitPrice = stopPrice - limitPriceDist(gen);
    }
    file << "ModifyStopLimit " << currentOrderId << " " << shares << " " << limitPrice << " " << stopPrice << std::endl;
    book->modifyStopLimitOrder(currentOrderId, shares, limitPrice, stopPrice);
}

void GenerateOrders::createOrders(int numberOfOrders)
{
    const auto outputPath = std::filesystem::path(__FILE__).parent_path() / "orders.txt";
    file.open(outputPath);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<double> probabilities = {0.05, 0.0, 0.20, 0.30, 0.0, 0.0, 0.15, 0.15, 0.0, 0.0, 0.15};
    std::vector<std::function<void()>> actions = {
        std::bind(&GenerateOrders::market, this),
        std::bind(&GenerateOrders::addLimit, this),
        std::bind(&GenerateOrders::cancelLimit, this),
        std::bind(&GenerateOrders::modifyLimit, this),
        std::bind(&GenerateOrders::addLimitMarket, this),
        std::bind(&GenerateOrders::addStop, this),
        std::bind(&GenerateOrders::cancelStop, this),
        std::bind(&GenerateOrders::modifyStop, this),
        std::bind(&GenerateOrders::addStopLimit, this),
        std::bind(&GenerateOrders::cancelStopLimit, this),
        std::bind(&GenerateOrders::modifyStopLimit, this),
    };

    std::partial_sum(probabilities.begin(), probabilities.end(), probabilities.begin());

    for (int i = 1; i <= numberOfOrders; ++i)
    {
        const double randNum = dis(gen);
        auto it = std::lower_bound(probabilities.begin(), probabilities.end(), randNum);
        int selectedAction = static_cast<int>(std::distance(probabilities.begin(), it));

        if (selectedAction >= static_cast<int>(actions.size())) {
            selectedAction = static_cast<int>(actions.size()) - 1;
        }

        actions[selectedAction]();
    }

    file.close();
    std::cout << "Orders written to orders.txt successfully!" << std::endl;
}

void GenerateOrders::createInitialOrders(int numberOfOrders, int centreOfBook)
{
    const auto outputPath = std::filesystem::path(__FILE__).parent_path() / "initialOrders.txt";
    std::ofstream file(outputPath);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    std::uniform_int_distribution<> sharesDist(1, 1000);
    std::normal_distribution<> limitPriceDist(centreOfBook, 50);

    for (int order = 1; order <= numberOfOrders; ++order) {
        int shares = sharesDist(gen);
        int limitPrice = static_cast<int>(limitPriceDist(gen));
        bool buyOrSell = limitPrice < centreOfBook;

        file << "AddLimit " << orderId << " " << buyOrSell << " " << shares << " " << limitPrice << std::endl;
        if (book != nullptr) {
            book->addLimitOrder(orderId, buyOrSell, shares, limitPrice);
        }
        ++orderId;
    }

    std::uniform_int_distribution<> stopLimitPriceDist(1, 5);
    std::uniform_int_distribution<> stopOrStopLimitDist(0, 1);

    for (int order = numberOfOrders + 1; order <= static_cast<int>(numberOfOrders * 1.1); ++order) {
        int shares = sharesDist(gen);
        int stopPrice = static_cast<int>(limitPriceDist(gen));
        bool buyOrSell = stopPrice > centreOfBook;
        bool stopOrStopLimit = stopOrStopLimitDist(gen);

        if (stopOrStopLimit)
        {
            file << "AddStop " << orderId << " " << buyOrSell << " " << shares << " " << stopPrice << std::endl;
            if (book != nullptr) {
                book->addStopOrder(orderId, buyOrSell, shares, stopPrice);
            }
        } else {
            int limitPrice = buyOrSell ? stopPrice + stopLimitPriceDist(gen) : stopPrice - stopLimitPriceDist(gen);
            file << "AddStopLimit " << orderId << " " << buyOrSell << " " << shares << " " << limitPrice << " " << stopPrice << std::endl;
            if (book != nullptr) {
                book->addStopLimitOrder(orderId, buyOrSell, shares, limitPrice, stopPrice);
            }
        }
        ++orderId;
    }

    file.close();
    std::cout << "Orders written to initialOrders.txt successfully!" << std::endl;
}
