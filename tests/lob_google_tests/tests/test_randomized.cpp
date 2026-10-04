#include <gtest/gtest.h>
#include "test_support.hpp"
#include <algorithm>
#include <random>
#include <vector>

using lob_test::verifyBookAvl;
using lob_test::verifyBookEdges;

TEST(RandomizedbookTest, ThousandsOfUniqueInsertionsMaintainAvlInvariants) {
    book book;
    std::vector<int> prices;

    for (int p = 1; p <= 2000; ++p) {
        prices.push_back(p * 2);
    }

    std::mt19937 rng(20261003);
    std::shuffle(prices.begin(), prices.end(), rng);

    int id = 1;
    for (const int price : prices) {
        book.AddLimitOrder(id++, true, 10 + (price % 37), price);

        ASSERT_NO_THROW(verifyBookAvl(book))
            << "Failure after inserting price " << price;
        ASSERT_NO_THROW(verifyBookEdges(book))
            << "Edge failure after inserting price " << price;
    }

    EXPECT_NE(book.getHighestBuy(), nullptr);
}

TEST(RandomizedbookTest, RepeatedSamePriceAggregationKeepsCorrectTotal) {
    book book;
    constexpr int orderCount = 1000;

    long long expectedShares = 0;
    for (int i = 1; i <= orderCount; ++i) {
        const int shares = (i % 17) + 1;
        expectedShares += shares;
        book.AddLimitOrder(i, true, shares, 100);
    }

    ASSERT_NE(book.getHighestBuy(), nullptr);
    EXPECT_EQ(book.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_EQ(book.getHighestBuy()->get_size(), orderCount);
    EXPECT_EQ(book.getHighestBuy()->get_totalshares(), expectedShares);
}
