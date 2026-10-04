#include <gtest/gtest.h>
#include "test_support.hpp"

TEST(LimitTest, LimitCreatedByBookStoresPriceAndSide) {
    book bookRef;

    bookRef.AddLimitOrder(1, true, 100, 100);
    bookRef.AddLimitOrder(2, false, 100, 105);

    ASSERT_NE(bookRef.getHighestBuy(), nullptr);
    ASSERT_NE(bookRef.getLowestSell(), nullptr);

    EXPECT_EQ(bookRef.getHighestBuy()->get_limitPrice(), 100);
    EXPECT_TRUE(bookRef.getHighestBuy()->getbuyorsell());
    EXPECT_EQ(bookRef.getLowestSell()->get_limitPrice(), 105);
    EXPECT_FALSE(bookRef.getLowestSell()->getbuyorsell());
}

TEST(LimitTest, NewPriceLevelStartsWithOneOrder) {
    book bookRef;
    bookRef.AddLimitOrder(1, true, 100, 100);

    ASSERT_NE(bookRef.getHighestBuy(), nullptr);
    EXPECT_EQ(bookRef.getHighestBuy()->get_size(), 1);
    EXPECT_EQ(bookRef.getHighestBuy()->get_totalshares(), 100);
    ASSERT_NE(bookRef.getHighestBuy()->get_headOrder(), nullptr);
    ASSERT_NE(bookRef.getHighestBuy()->get_tailOrder(), nullptr);
    EXPECT_EQ(bookRef.getHighestBuy()->get_headOrder()->get_idNumber(), 1);
    EXPECT_EQ(bookRef.getHighestBuy()->get_tailOrder()->get_idNumber(), 1);
}

TEST(LimitTest, AppendingOrdersPreservesFifoAndAggregateQuantity) {
    book bookRef;
    bookRef.AddLimitOrder(1, true, 100, 100);
    bookRef.AddLimitOrder(2, true, 200, 100);
    bookRef.AddLimitOrder(3, true, 300, 100);

    ASSERT_NE(bookRef.getHighestBuy(), nullptr);
    EXPECT_EQ(bookRef.getHighestBuy()->get_size(), 3);
    EXPECT_EQ(bookRef.getHighestBuy()->get_totalshares(), 600);
    EXPECT_EQ(bookRef.getHighestBuy()->get_headOrder()->get_idNumber(), 1);
    EXPECT_EQ(bookRef.getHighestBuy()->get_tailOrder()->get_idNumber(), 3);
}

TEST(LimitTest, CancellingOneOrderUpdatesLevelAggregate) {
    book bookRef;
    bookRef.AddLimitOrder(1, true, 125, 100);
    bookRef.AddLimitOrder(2, true, 75, 100);
    bookRef.AddLimitOrder(3, true, 300, 100);

    bookRef.CancelLimitOrder(2);

    ASSERT_NE(bookRef.getHighestBuy(), nullptr);
    EXPECT_EQ(bookRef.getHighestBuy()->get_size(), 2);
    EXPECT_EQ(bookRef.getHighestBuy()->get_totalshares(), 425);
    EXPECT_EQ(bookRef.getHighestBuy()->get_headOrder()->get_idNumber(), 1);
    EXPECT_EQ(bookRef.getHighestBuy()->get_tailOrder()->get_idNumber(), 3);
}

TEST(LimitTest, DifferentPriceLevelsRemainSeparate) {
    book bookRef;
    bookRef.AddLimitOrder(1, true, 100, 100);
    bookRef.AddLimitOrder(2, true, 250, 105);

    ASSERT_NE(bookRef.getHighestBuy(), nullptr);
    EXPECT_EQ(bookRef.getHighestBuy()->get_limitPrice(), 105);
    EXPECT_EQ(bookRef.getHighestBuy()->get_totalshares(), 250);

    limit* root = lob_test::rootOf(bookRef.getHighestBuy());
    ASSERT_NE(root, nullptr);
    ASSERT_NE(lob_test::findLimit(root, 100), nullptr);
    EXPECT_EQ(lob_test::findLimit(root, 100)->get_totalshares(), 100);
}
